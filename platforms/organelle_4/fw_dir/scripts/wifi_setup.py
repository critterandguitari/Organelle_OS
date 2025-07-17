import os
import imp
import sys
import time
import threading
import subprocess

# Setup environment
user_dir = os.getenv("USER_DIR", "/sdcard")
fw_dir = os.getenv("FW_DIR")

# Import UI module
current_dir = os.path.dirname(os.path.abspath(__file__))
og = imp.load_source('og', current_dir + '/og.py')

# WiFi connection states
NOT_CONNECTED = 0
CONNECTING = 1
CONNECTED = 2
DISCONNECTING = 3
CONNECTION_ERROR = 4
AP_MODE = 5

# WiFi state variables
state = NOT_CONNECTED
connecting_timer = 0
current_net = ''
ip_address = ''

# Web server states
WEB_SERVER_STOPPED = 0
WEB_SERVER_RUNNING = 1
web_server_state = WEB_SERVER_STOPPED

# Log file
log_file = user_dir + "/wifi_log.txt"

# UI elements
menu = og.Menu()
network_menu = og.Menu()

# === WiFi Control Functions ===

def run_cmd(cmd):
    """Run command with sudo, return output or False"""
    cmd = "sudo " + cmd
    try:
        return subprocess.check_output(['bash', '-c', cmd], close_fds=True, text=True)
    except:
        return False

def run_cmd_check(cmd):
    """Run command with sudo, return True/False on success/failure"""
    cmd = "sudo " + cmd
    try:
        subprocess.check_output(['bash', '-c', cmd], close_fds=True, text=True)
        return True
    except:
        return False

def start_web_server():
    """Start the web server"""
    global web_server_state
    run_cmd('systemctl start ogweb')
    web_server_state = WEB_SERVER_RUNNING

def stop_web_server():
    """Stop the web server"""
    global web_server_state
    run_cmd('systemctl stop ogweb')
    web_server_state = WEB_SERVER_STOPPED

def wifi_connected():
    """Check if WiFi is connected and update network info"""
    global ip_address, current_net
    try:
        result = subprocess.run(['wpa_cli', '-i', 'wlan0', 'status'], 
                              capture_output=True, text=True, timeout=5)
        if result.returncode == 0:
            wifi_info = result.stdout.splitlines()
            if any("ip_address" in s for s in wifi_info):
                update_network_info(wifi_info)
                return True
    except:
        pass
    return False

def update_network_info(info):
    """Update IP address and current network from wpa_cli output"""
    global ip_address, current_net
    try:
        ip_address = [s for s in info if s.startswith('ip_address')][0][11:]
    except:
        pass
    try:
        current_net = [s for s in info if s.startswith('ssid')][0][5:]
    except:
        pass

def initialize_state():
    """Initialize WiFi and web server states on startup"""
    global state, web_server_state
    
    # Check initial WiFi state
    if wifi_connected():
        state = CONNECTED
    else:
        state = NOT_CONNECTED

    # Check initial web server state
    if run_cmd_check('systemctl status ogweb'):
        web_server_state = WEB_SERVER_RUNNING
    else:
        web_server_state = WEB_SERVER_STOPPED

def disconnect_all():
    """Disconnect from WiFi and stop web server"""
    global state, ip_address
    state = DISCONNECTING
    ip_address = ""  # Clear IP address
    try:
        subprocess.run(["sudo", "nmcli", "device", "disconnect", "wlan0"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error disconnecting WiFi: {e}")

    stop_web_server()
    state = NOT_CONNECTED

def list_ssids():
    """Get list of available WiFi networks with fresh scan"""
    try:
        # Force a fresh scan first
        subprocess.run(
            ["sudo", "nmcli", "device", "wifi", "rescan"],
            capture_output=True, timeout=10
        )
        
        # Get the fresh results
        result = subprocess.run(
            ["nmcli", "-t", "-f", "SSID", "dev", "wifi", "list"],
            capture_output=True, text=True, check=True, timeout=15
        )
        ssids = {ssid.strip() for ssid in result.stdout.splitlines() if ssid.strip()}
        return sorted(ssids)
    except subprocess.CalledProcessError as e:
        print(f"Error listing WiFi networks: {e}")
        return []
    except subprocess.TimeoutExpired:
        print("Timed out waiting for nmcli to scan networks")
        return []

def connect_nopw(ssid):
    """Connect to WiFi network without password"""
    global state, connecting_timer, current_net
    
    state = CONNECTING
    connecting_timer = 0
    current_net = ssid

    # Start log
    run_cmd("echo WIFI LOG > " + log_file)
    
    cmd = ["sudo", "nmcli", "device", "wifi", "connect", ssid]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=15)
        state = CONNECTED
        print(f"Connected to {ssid}")
        return True
    except subprocess.CalledProcessError as e:
        state = CONNECTION_ERROR
        error_output = e.stderr if e.stderr else e.stdout
        print(f"Error connecting to {ssid}: {error_output}")
        return False

def connect(ssid, password):
    """Connect to WiFi network with password"""
    global state, connecting_timer, current_net
    
    state = CONNECTING
    connecting_timer = 0
    current_net = ssid

    # Start log
    run_cmd("echo WIFI LOG > " + log_file)

    cmd = ["sudo", "nmcli", "device", "wifi", "connect", ssid, "password", password]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=15)
        state = CONNECTED
        print(f"Connected to {ssid}")
        return True
    except subprocess.CalledProcessError as e:
        state = CONNECTION_ERROR
        error_output = e.stderr if e.stderr else e.stdout
        print(f"Error connecting to {ssid}: {error_output}")
        return False
    except subprocess.TimeoutExpired:
        state = CONNECTION_ERROR
        print(f"Timeout connecting to {ssid}")
        return False

def start_ap_mode():
    """Start WiFi Access Point mode"""
    global state, connecting_timer, current_net
    
    print("Starting AP mode...")
    og.alert("Starting AP mode...")
    
    # Disconnect first to be sure
    try:
        subprocess.run(["sudo", "nmcli", "device", "disconnect", "wlan0"], 
                      capture_output=True, timeout=10)
    except:
        pass  # Ignore errors if already disconnected
    
    # Start AP mode
    try:
        result = subprocess.run([
            "sudo", "nmcli", "device", "wifi", "hotspot", 
            "ssid", "ORGANELLE", "password", "coolmusic"
        ], capture_output=True, text=True, check=True, timeout=15)
        
        state = AP_MODE
        print("AP mode started successfully")
        return True
    except subprocess.CalledProcessError as e:
        state = CONNECTION_ERROR
        error_output = e.stderr if e.stderr else e.stdout
        print(f"Error starting AP mode: {error_output}")
        return False
    except subprocess.TimeoutExpired:
        state = CONNECTION_ERROR
        print("Timeout starting AP mode")
        return False
    """Connect to WiFi network with password"""
    
    state = CONNECTING
    connecting_timer = 0
    current_net = ssid

    # Start log
    run_cmd("echo WIFI LOG > " + log_file)

    cmd = ["sudo", "nmcli", "device", "wifi", "connect", ssid, "password", password]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=15)
        state = CONNECTED
        print(f"Connected to {ssid}")
        return True
    except subprocess.CalledProcessError as e:
        state = CONNECTION_ERROR
        error_output = e.stderr if e.stderr else e.stdout
        print(f"Error connecting to {ssid}: {error_output}")
        return False

# === UI Functions ===

def quit():
    """Exit the application"""
    og.end_app()

def disconnect():
    """Disconnect from WiFi"""
    print("WiFi disconnect all")
    og.alert("Disconnecting...")
    disconnect_all()
    update_menu()

def start_ap():
    """Start AP mode"""
    print("Starting AP mode")
    start_ap_mode()
    update_menu()

def start_web():
    """Start web server"""
    print("Start web server")
    start_web_server()
    update_menu()

def stop_web():
    """Stop web server"""
    print("Stop web server")
    stop_web_server()
    update_menu()

def scan_and_connect():
    """Scan for networks and let user select one to connect to"""
    try:
        og.alert("Scanning...")
        ssids = list_ssids()
       
        network_menu.header = "Select Network"
        network_menu.reset()
        network_menu.items = []
        
        if not ssids or len(ssids) < 1:
            network_menu.items.append(["No Networks Found", lambda: None])
        else:
            # Add each SSID as a menu item
            for ssid in ssids:
                network_menu.items.append([ssid, lambda s=ssid: connect_to_network(s)])
            
        # Add back option
        network_menu.items.append(['< Back', network_menu.back])
        
        # Show the menu
        network_menu.perform()
            
    except Exception as e:
        print(f"Failed scan or connect: {str(e)}")

def connect_to_network(ssid):
    """Connect to a specific network, prompting for password if needed"""
    
    try:
        print(f"Connecting to {ssid} attempting with no password...")
        og.alert("Connecting...")
        success = connect_nopw(ssid)
        
        if success:
            print(f"Connected to {ssid}")
            network_menu.back()
            update_menu()
            return
    except Exception as e:
        print(f"Failed connect no password: {str(e)}")

    try:
        # Need password from user
        pwd_entry = og.PasswordEntry(f"Password for {ssid}", max_length=64)
        password = pwd_entry.perform()
        
        if (password is None) or (password == ""):
            # User cancelled password entry
            network_menu.back()
            update_menu()
            return
        
        print(f"Connecting to {ssid}...")
        og.alert("Connecting...")
        
        # Attempt to connect
        success = connect(ssid, password)
        
        if success:
            print(f"Connected to {ssid}")
        else:
            print("Connection failed")
            
    except Exception as e:
        print(f"Failed connect with password: {str(e)}")

    network_menu.back()
    update_menu()

def network_menu_action():
    """Handle network menu selection"""
    if state != CONNECTED:
        scan_and_connect()

def build_main_menu():
    """Build the main menu based on current state"""
    menu.items = []
    
    print(f"Build menu state: {state} web server state: {web_server_state}")
    
    # Update menu based on WiFi state
    if state == CONNECTING:
        menu.header = 'Not Connected'
    elif state == CONNECTED:
        menu.header = 'Connected ' + current_net
        
        # Show IP address if available
        if ip_address:
            menu.items.append([f'IP: {ip_address}', lambda: None])
        menu.items.append(['Disconnect', disconnect])
        
        # Web server control
        if web_server_state == WEB_SERVER_RUNNING:
            menu.items.append(['Stop Web Server', stop_web])
        else:
            menu.items.append(['Start Web Server', start_web])
    elif state == AP_MODE:
        menu.header = 'AP Mode (ORGANELLE)'
        
        # Show AP mode IP (typically 10.42.0.1 for NetworkManager hotspot)
        menu.items.append(['IP: 10.42.0.1', lambda: None])
        menu.items.append(['Stop AP Mode', disconnect])
        
        # Web server control in AP mode
        if web_server_state == WEB_SERVER_RUNNING:
            menu.items.append(['Stop Web Server', stop_web])
        else:
            menu.items.append(['Start Web Server', start_web])
    elif state == DISCONNECTING:
        menu.header = 'Not Connected'
    elif state == CONNECTION_ERROR:
        menu.header = 'Problem Connecting'
        menu.items.append(['Select Network   >', network_menu_action])
        menu.items.append(['Start AP Mode', start_ap])
    else:
        menu.header = 'Not Connected'
        menu.items.append(['Select Network   >', network_menu_action])
        menu.items.append(['Start AP Mode', start_ap])
    
    # Home/quit
    menu.items.append(['< Home', quit])

def update_menu():
    """Update menu based on connection status"""
    try:
        build_main_menu()
        og.redraw_flag = True
    except Exception as e:
        print(f"Error: {str(e)}")

def check_status():
    """Background connection checker"""
    while True:
        time.sleep(1)
        update_menu()

# === Main Application ===

def main():
    """Main application entry point"""
    # Start the UI
    og.start_app()
    
    # Initialize WiFi state
    initialize_state()
    
    # Build main menu
    update_menu()
     
    # Enter menu
    menu.perform()

if __name__ == "__main__":
    main()
