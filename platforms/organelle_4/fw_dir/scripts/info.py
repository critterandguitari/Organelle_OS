import os
import imp
import sys
import time
import threading
import subprocess
import socket

print ("Starting INFO script")

# usb or sd card
user_dir = os.getenv("USER_DIR", "/usbdrive")
patch_dir = os.getenv("PATCH_DIR", "/usbdrive/Patches")
fw_dir = os.getenv("FW_DIR")

# imports
current_dir = os.path.dirname(os.path.abspath(__file__))
og = imp.load_source('og', current_dir + '/og.py')
wifi = imp.load_source('wifi_control', current_dir + '/wifi_control.py')

ssid = "not connected"
ip_address = "not connected"

wifi.initialize_state()

def check_wifi():
    global ssid, ip_address
    if wifi.wifi_connected() :
        ssid = wifi.current_net
        ip_address = wifi.ip_address

def check_vnc():
    cmd = "pgrep vncserver"
    try:
        subprocess.check_output(['bash', '-c', cmd], close_fds=True)
        ret = True
    except: 
        ret = False
    return ret

# returns output if exit code 0, NA otherwise
def run_cmd(cmd) :
    ret = 'None'
    try:
        ret = subprocess.check_output(['bash', '-c', cmd], close_fds=True)
    except: pass
    return ret

def check_status():
    global info, ssid, ip_address
    while True:
        cpu_load = run_cmd("top -d 1 -n 2 -b -p `ps -C pd -o %p | tail -1` | tail -1 | awk '{print $9}'").rstrip()
        if cpu_load == '' : cpu_load = 0
        else : cpu_load = int(float(cpu_load))
        # cpu load is of Pd only... just add 5% for the other stuff
        cpu_load += 5
        info.items[0] = "CPU: " + str(cpu_load) + " %"
        check_wifi()
        info.items[2] = "IP: " + ip_address
        info.items[4] = "  " + ssid
        og.redraw_flag = True

info = og.InfoList()

# start it up
og.start_app()

# get info
cpu = "CPU: ..."
usbdrive = "USB Drive: " + run_cmd("grep usbdrive /proc/mounts | awk '{print $1}' | sed -e 's/\/dev\///'")
midi_dev =  run_cmd("aplaymidi -l | awk '{if (NR==2) print $2}'")
if (midi_dev == ""): midi_dev = 'None'
version = run_cmd("cat " + fw_dir + "/version")
build_tag = run_cmd("cat " + fw_dir + "/buildtag")
version = "Version: " + version + build_tag
patch_dir = "  " + patch_dir.split(user_dir + "/", 1).pop()
user_dir = "  " + user_dir
patch = "  " + run_cmd("ls /tmp/curpatchname")
host_name = "  " + run_cmd("ps aux | grep 'avahi.*running' | awk 'NR==1{print $13}' | sed 's/\[//' | sed 's/]//'")
if check_vnc() : vnc_server = "  Running"
else : vnc_server = "  Not Running"
check_wifi()

#info.items = [usbdrive, midi_dev, version, "Patch Folder:", patch_dir, "User Root:", user_dir, ip_address, "Host Name:",host_name]
info.items = [
cpu,
usbdrive, 
"IP: " + ip_address, 
"WiFi Network:",
"  " + ssid,
"Host Name:",
host_name,
"VNC Server:",
vnc_server,
"Patch: ",
patch,
"Patch Folder:", 
patch_dir, 
"User Root:", 
user_dir, 
version, 
]

info.header='INFO press to exit.'

# bg thread
menu_updater = threading.Thread(target=check_status)
menu_updater.daemon = True # stop the thread when we exit


# start thread to update connection status
menu_updater.start()

# enter menu
info.draw()
og.redraw_flag = True
info.perform()

og.end_app()

