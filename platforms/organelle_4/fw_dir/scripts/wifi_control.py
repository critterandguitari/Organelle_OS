import os
import subprocess

log_file = os.getenv("USER_DIR", "/sdcard") + "/wifi_log.txt"

# states
NOT_CONNECTED = 0
CONNECTING = 1
CONNECTED = 2
DISCONNECTING = 3
CONNECTION_ERROR = 4

# current state
state = NOT_CONNECTED
connecting_timer = 0

# webserver
WEB_SERVER_STOPPED = 0
WEB_SERVER_RUNNING = 1
web_server_state = WEB_SERVER_STOPPED 

current_net = ''
ip_address = ''

# returns output if exit code 0, false otherwise
# most of the commands need sudo
def run_cmd(cmd) :
    cmd = "sudo " + cmd
    ret = False
    try:
        ret = subprocess.check_output(['bash', '-c', cmd], close_fds=True, text=True)
    except: pass
    return ret

# for commands without sudo
def run_cmd_nosudo(cmd) :
    ret = False
    try:
        ret = subprocess.check_output(['bash', '-c', cmd], close_fds=True, text=True)
    except: pass
    return ret

# returns true or false on exit status
def run_cmd_check(cmd) :
    cmd = "sudo " + cmd
    ret = False
    try:
        subprocess.check_output(['bash', '-c', cmd], close_fds=True, text=True)
        ret = True
    except: pass
    return ret

def start_web_server():
    global web_server_state
#    run_cmd('systemctl start cherrypy')
    web_server_state = WEB_SERVER_RUNNING

def stop_web_server():
    global web_server_state
#    run_cmd('systemctl stop cherrypy')
    web_server_state = WEB_SERVER_STOPPED

# true or false connected with ip address
# updates ip and current network when connected
def wifi_connected():
    global ip_address, current_net
    ret = False
    try :
        # Suppress both stdout and stderr, only care if command succeeds
        result = subprocess.run(['sudo', 'wpa_cli', '-i', 'wlan0', 'status'], 
                              capture_output=True, 
                              text=True, 
                              timeout=5)
        if result.returncode == 0:
            wifi_info = result.stdout.splitlines()
            if (any("ip_address" in s for s in wifi_info)):
                update_network_info(wifi_info)
                ret = True
    except : pass
    return ret

# helper
def update_network_info(info):
    global ip_address, current_net
    try : ip_address = [s for s in info if s.startswith('ip_address')][0][11:]
    except : pass
    try : current_net = [s for s in info if s.startswith('ssid')][0][5:]
    except : pass

# get initial connection state and ip and ssid
def initialize_state():
    global state, web_server_state
    
    # wifi state on startup
    if wifi_connected() : 
        state = CONNECTED
    else : state = NOT_CONNECTED

    # web server state on startup
 #   if (run_cmd_check('systemctl status cherrypy')) : web_server_state = WEB_SERVER_RUNNING
 #   else : web_server_state = WEB_SERVER_STOPPED

# shut everything off
def disconnect_all() :
    global state
    state = DISCONNECTING
    try:
        subprocess.run(["sudo", "nmcli", "device", "disconnect", "wlan0"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error disconnecting WiFi: {e}")

    stop_web_server()
    state = NOT_CONNECTED

def list_ssids():
    try:
        result = subprocess.run(
            ["nmcli", "-t", "-f", "SSID", "dev", "wifi"],
            capture_output=True,
            text=True,
            check=True,
            timeout=15
        )
        ssids = {ssid.strip() for ssid in result.stdout.splitlines() if ssid.strip()}
        return sorted(ssids)
    except subprocess.CalledProcessError as e:
        print(f"Error listing WiFi networks: {e}")
        return []
    except subprocess.TimeoutExpired:
        print("Timed out waiting for nmcli to list networks")
        return []

def connect_nopw(ssid) :
    global state, connecting_timer, current_net
  
    # update state
    state = CONNECTING
    connecting_timer = 0
    current_net = ssid

    # restart log
    run_cmd("echo WIFI LOG > " + log_file)
    cmd = ["sudo", "nmcli", "device", "wifi", "connect", ssid]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=15)
        # If we reach here, the command succeeded
        state = CONNECTED
        print(f"connected {state}")
        return True
    except subprocess.CalledProcessError as e:
        # Command failed
        state = CONNECTION_ERROR
        error_output = e.stderr if e.stderr else e.stdout
        print(f"Error connecting to {ssid}: {error_output}")
        return False

def connect(ssid, password) :
    global state, connecting_timer, current_net
  
    # update state
    state = CONNECTING
    connecting_timer = 0
    current_net = ssid

    # restart log
    run_cmd("echo WIFI LOG > " + log_file)

    cmd = ["sudo", "nmcli", "device", "wifi", "connect", ssid, "password", password]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=15)
        # If we reach here, the command succeeded
        state = CONNECTED
        print(f"connected {state}")
        return True
    except subprocess.CalledProcessError as e:
        # Command failed
        state = CONNECTION_ERROR
        error_output = e.stderr if e.stderr else e.stdout
        print(f"Error connecting to {ssid}: {error_output}")
        return False


