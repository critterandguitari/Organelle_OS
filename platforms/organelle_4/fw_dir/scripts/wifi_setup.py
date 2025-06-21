import os
import imp
import sys
import time
import threading
import subprocess

# usb or sd card
user_dir = os.getenv("USER_DIR", "/usbdrive")
fw_dir = os.getenv("FW_DIR")

# imports
current_dir = os.path.dirname(os.path.abspath(__file__))
og = imp.load_source('og', current_dir + '/og.py')
wifi = imp.load_source('wifi_control', current_dir + '/wifi_control.py')

wifi.log_file = user_dir + "/wifi_log.txt"

# UI elements
menu = og.Menu()
banner = og.Alert()

# lock for updating menu
menu_lock = threading.Lock()

# quits but doesn't return to main menu
# for vnc start / stop, keeps the starting / stopping message up until mother is restarted
def quit_no_return():
    og.osc_server.free()
    exit()

def quit():
    og.end_app()

# stores possible networks
# used to build wifi menu
# contains connect callback
class WifiNet :
    ssid = ''
    pw = ''
    def connect (self):
        wifi.connect(self.ssid, self.pw)
        update_menu()
        og.redraw_flag = True

def disconnect():
    print ("wifi disconnect all")
    wifi.disconnect_all()
    update_menu()
    og.redraw_flag = True

def start_vnc():
    #cmd = fw_dir + "/scripts/vnc-start.sh"
    #try:
    #    ret = subprocess.check_output(['bash', '-c', cmd], close_fds=True)
    #except: pass
    #quit_no_return()
    pass

def stop_vnc():
    #cmd = fw_dir + "/scripts/vnc-stop.sh"
    #try:
    #    ret = subprocess.check_output(['bash', '-c', cmd], close_fds=True)
    #except: pass
    #quit_no_return()
    pass

def check_vnc():
    #cmd = "pgrep vncserver"
    #try:
    #    subprocess.check_output(['bash', '-c', cmd], close_fds=True)
    #    ret = True
    #except: 
    #    ret = False
    return False # ret

def start_web():
    print ("start web")
    wifi.start_web_server()
    update_menu()
    og.redraw_flag = True

def stop_web():
    print ("stop web")
    wifi.stop_web_server()
    update_menu()
    og.redraw_flag = True

def start_ap():
    print ("start ap")
    wifi.start_ap_server()
    update_menu()
    og.redraw_flag = True

def stop_ap():
    print ("stop ap")
    wifi.stop_ap_server()
    update_menu()
    og.redraw_flag = True

# update menu based on connection status
def update_menu():
    dots = ['.','..','...','....']
    menu_lock.acquire()
    try :
        # update wifi network labels
        if (wifi.state == wifi.CONNECTING) : 
            menu.header = 'Connecting'+dots[wifi.connecting_timer % 4]
        elif (wifi.state == wifi.CONNECTED) : 
            menu.header = 'Connected ' + wifi.current_net
        elif (wifi.state == wifi.DISCONNECTING) : 
            menu.header = 'Disconnecting..'
        elif (wifi.state == wifi.CONNECTION_ERROR) : 
            menu.header = 'Problem Connecting'
        else : 
            menu.header = 'Not Connected'
        
        # update webserver menu entry
        if (wifi.web_server_state == wifi.WEB_SERVER_RUNNING) :
            update_web_server_menu_entry(True)
        else :
            update_web_server_menu_entry(False)
    
        # update webserver menu entry
        if (wifi.ap_state == wifi.AP_RUNNING) :
            update_ap_menu_entry(True)
        else :
            update_ap_menu_entry(False)
    finally :
        menu_lock.release()

def update_web_server_menu_entry(stat):
    if (stat) :
        label = 'Stop Web Server'
        action = stop_web
    else :
        label = 'Start Web server'
        action = start_web
    for i in range(len(menu.items)) :
        try :
            if (menu.items[i][2]['type'] == 'web_server_control') :
                menu.items[i][0] = label
                menu.items[i][1] = action
        except :
            pass

def update_ap_menu_entry(stat):
    if (stat) :
        label = 'Stop AP'
        action = stop_ap
    else :
        label = 'Start AP'
        action = start_ap
    for i in range(len(menu.items)) :
        try :
            if (menu.items[i][2]['type'] == 'ap_control') :
                menu.items[i][0] = label
                menu.items[i][1] = action
        except :
            pass

# bg connection checker
def check_status():
    while True:
        time.sleep(1)
        wifi.update_state()
        update_menu()
        og.redraw_flag = True

def non():
    pass

def test_password_entry():
    pwd_entry = og.PasswordEntry("WiFi Password", max_length=20)
    result = pwd_entry.perform()
    print(result)

# build main menu
menu.items = []
menu.header='Not Connected'

# start it up
og.start_app()

menu.items.append(['Start Web Server', non, {'type':'web_server_control'}])
menu.items.append(['Start AP', non, {'type':'ap_control'}])
if check_vnc() :  menu.items.append(['Stop VNC', stop_vnc])
else : menu.items.append(['Start VNC', start_vnc])
menu.items.append(['Turn Wifi Off', disconnect])
menu.items.append(["Enter Password", test_password_entry]),
menu.items.append(['< Home', quit])
menu.selection = 0

# bg thread
menu_updater = threading.Thread(target=check_status)
menu_updater.daemon = True # stop the thread when we exit

wifi.initialize_state()
update_menu()
og.redraw_flag = True

# start thread to update connection status
menu_updater.start()
 
# enter menu
menu.perform()

