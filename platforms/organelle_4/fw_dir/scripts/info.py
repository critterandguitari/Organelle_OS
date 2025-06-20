import os
import imp
import sys
import time
import threading
import subprocess
import socket
import psutil 

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

def get_cpu_temp():
    try:
        result = subprocess.check_output(['vcgencmd', 'measure_temp'], text=True)
        temp = result.strip().replace('temp=', '').replace("'C", ' C')
        return temp
    except:
        return "-- °C"

def check_wifi():
    global ssid, ip_address
    if wifi.wifi_connected() :
        ssid = wifi.current_net
        ip_address = wifi.ip_address

def check_vnc():
    cmd = "pgrep vncserver"
    try:
        subprocess.check_output(['bash', '-c', cmd], close_fds=True, text=True)
        ret = True
    except: 
        ret = False
    return ret

# returns output if exit code 0, NA otherwise
def run_cmd(cmd) :
    ret = 'None'
    try:
        ret = subprocess.check_output(['bash', '-c', cmd], close_fds=True, text=True)
    except: pass
    return ret


import time
import psutil

def check_status():
    global info, ssid, ip_address
    while True:
        try:
            # Get per-CPU usage as a list
            cpu_percents = psutil.cpu_percent(interval=0.1, percpu=True)
            
            # Format each core's usage
            cpu_strings = [f"{int(cpu)}" for i, cpu in enumerate(cpu_percents)]
            
            info.items[0] = "CPU: " + " ".join(cpu_strings)
            
            # Get temperature
            info.items[1] = "TEMP: " + get_cpu_temp()

            check_wifi()
            info.items[3] = "IP: " + ip_address
            info.items[5] = "  " + ssid
            og.redraw_flag = True
            
        except Exception as e:
            print(f"Error in check_status: {e}")
            info.items[0] = "CPU: -- %"
        
        time.sleep(1)

info = og.InfoList()

# start it up
og.start_app()

# get info
cpu = "CPU: ..."
temp = "TEMP: ..."
usbdrive = "USB Drive: " + run_cmd("grep usbdrive /proc/mounts | awk '{print $1}' | sed -e 's/\/dev\///'")
midi_dev =  run_cmd("aplaymidi -l | awk '{if (NR==2) print $2}'")
if (midi_dev == ""): midi_dev = 'None'
version = run_cmd("cat " + fw_dir + "/version")
ogmodel = run_cmd("cat " + fw_dir + "/ogmodel")
build_tag = run_cmd("cat " + fw_dir + "/buildtag")
version = "Version: " + version + build_tag
ogmodel = "Model: " + ogmodel
patch_dir = "  " + patch_dir.split(user_dir + "/", 1).pop()
user_dir = "  " + user_dir
patch = "  " + run_cmd("ls /tmp/curpatchname")
#host_name = "  " + run_cmd("ps aux | grep 'avahi.*running' | awk 'NR==1{print $13}' | sed 's/\[//' | sed 's/]//'")
if check_vnc() : vnc_server = "  Running"
else : vnc_server = "  Not Running"
check_wifi()

#info.items = [usbdrive, midi_dev, version, "Patch Folder:", patch_dir, "User Root:", user_dir, ip_address, "Host Name:",host_name]
info.items = [
cpu,
temp,
usbdrive, 
"IP: " + ip_address, 
"WiFi Network:",
"  " + ssid,
#"Host Name:",
#host_name,
"VNC Server:",
vnc_server,
"Patch: ",
patch,
"Patch Folder:", 
patch_dir, 
"User Root:", 
user_dir, 
version, 
ogmodel, 
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

