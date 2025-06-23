import os
import imp
import sys
import time
import threading
import subprocess

# setup env
user_dir = os.getenv("USER_DIR", "/sdcard")
fw_dir = os.getenv("FW_DIR")

# imports
current_dir = os.path.dirname(os.path.abspath(__file__))
og = imp.load_source('og', current_dir + '/og.py')
wifi = imp.load_source('wifi_control', current_dir + '/wifi_control.py')

# log file
wifi.log_file = user_dir + "/wifi_log.txt"

# UI elements
menu = og.Menu()
network_menu = og.Menu()

def quit():
    og.end_app()

def nonf():
    pass

def disconnect():
    print ("wifi disconnect all")
    og.alert("Disconnecting...")
    wifi.disconnect_all()
    update_menu()

def start_vnc():
    # start vnc
    pass

def stop_vnc():
    # stop vnc
    pass

def check_vnc():
    # check vnc
    return False # ret

def start_web():
    print ("start web")
    wifi.start_web_server()
    update_menu()

def stop_web():
    print ("stop web")
    wifi.stop_web_server()
    update_menu()

def scan_and_connect():
    """Scan for networks and let user select one to connect to"""
    try:
        og.alert("Scanning...")
        # Get list of available SSIDs
        ssids = wifi.list_ssids()
       
        # Create menu with available networks
        network_menu.header = "Select Network"
        network_menu.reset()
        network_menu.items = []
        if not ssids or len(ssids) < 1:
            network_menu.items.append(["No Networks Found", nonf])
        else :
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
        print(f"Connecting to {ssid} attempting with no pw...")
       
        og.alert("Connecting...")
        success = wifi.connect_nopw(ssid)
        
        if success:
            print(f"Connected to {ssid}")
            network_menu.back()
            update_menu()
            return
    except Exception as e:
        print(f"Failed connect no pw: {str(e)}")

    try:
        # otherwise need password from user
        pwd_entry = og.PasswordEntry(f"Password for {ssid}", max_length=64)
        password = pwd_entry.perform()
        
        if (password is None) or (password == ""):
            # User cancelled password entry
            return
        
        #def do_con():
        print(f"Connecting to {ssid}...")
        
        og.alert("Connecting...")
        # Attempt to connect
        success = wifi.connect(ssid, password)
        
        if success:
            print(f"Connected to {ssid}")
        else:
            print("Connection failed")
            
        # Update menu to reflect new state
        #update_menu()

        #hreading.Thread(target=do_con, daemon=True).start()
    except Exception as e:
        print(f"Failed connect with pw: {str(e)}")


    network_menu.back()
    update_menu()

def network_menu_action():
    if not wifi.state == wifi.CONNECTED:
        scan_and_connect()

def build_main_menu():
    """Build the main menu based on current state"""
    menu.items = []
    
    print(f"build menu state: {wifi.state} web server state: {wifi.web_server_state}")
    # update wifi network labels
    if (wifi.state == wifi.CONNECTING) : 
        menu.header = 'Not Connected'
    elif (wifi.state == wifi.CONNECTED) : 
        menu.header = 'Connected ' + wifi.current_net
        menu.items.append(['Disconnect', disconnect]) 
        # Web server control
        if (wifi.web_server_state == wifi.WEB_SERVER_RUNNING) :
            menu.items.append(['Stop Web Server', stop_web, {'type':'web_server_control'}])
        else:
            menu.items.append(['Start Web Server', start_web, {'type':'web_server_control'}])
    
    elif (wifi.state == wifi.DISCONNECTING) : 
        menu.header = 'Not Connected'
    elif (wifi.state == wifi.CONNECTION_ERROR) : 
        menu.header = 'Problem Connecting'
        menu.items.append(['Select Network   >', network_menu_action])
    else : 
        menu.header = 'Not Connected'
        menu.items.append(['Select Network   >', network_menu_action])
       # VNC control
    #if check_vnc():
    #    menu.items.append(['Stop VNC', stop_vnc])
    #else:
    #    menu.items.append(['Start VNC', start_vnc])
    
    # Home/quit
    menu.items.append(['< Home', quit])
    
# update menu based on connection status
def update_menu():
    try :
        # Rebuild menu items based on current state
        build_main_menu()
        og.redraw_flag = True
    except Exception as e:
        print(f"Error: {str(e)}")

# bg connection checker
def check_status():
    while True:
        time.sleep(1)
        update_menu()

def test_password_entry():
    pwd_entry = og.PasswordEntry("WiFi Password", max_length=20)
    result = pwd_entry.perform()
    if result is not None:
        print(f"Password: {result}")
    else:
        print("Cancelled")

# start it up
og.start_app()

# build main menu
build_main_menu()
menu.header='Not Connected'

wifi.initialize_state()
update_menu()
 
# enter menu
menu.perform()
