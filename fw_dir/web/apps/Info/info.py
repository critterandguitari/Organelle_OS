import os
import imp
import sys
import time
import threading
import subprocess
import socket

# usb or sd card
fw_dir = os.getenv("FW_DIR", "/home/music/fw_dir")

# imports
#print fw_dir
wifi = imp.load_source('wifi_control', fw_dir + '/scripts/wifi_control.py')

ssid = "not connected"
ip_address = "not connected"

def check_wifi():
    global ssid, ip_address
    if wifi.wifi_connected() :
        ssid = wifi.current_net
        ip_address = wifi.ip_address

# returns output if exit code 0, NA otherwise
def run_cmd(cmd) :
    ret = 'None'
    try:
        ret = subprocess.check_output(['bash', '-c', cmd], close_fds=True)
    except: pass
    return ret

# get info
cpu = "CPU: " + str(100 - int(run_cmd("vmstat 1 2|tail -1|awk '{print $15}'"))) + " %"
usbdrive = "USB Drive: " + run_cmd("grep usbdrive /proc/mounts | awk '{print $1}' | sed -e 's/\/dev\///'")
midi_dev =  run_cmd("aplaymidi -l | awk '{if (NR==2) print $2}'")
if (midi_dev == ""): midi_dev = 'None'
version = run_cmd("cat " + fw_dir + "/version")
build_tag = run_cmd("cat " + fw_dir + "/buildtag")
version = "OS version: " + version + build_tag
patch = "  " + run_cmd("ls /tmp/curpatchname")
host_name = "  " + run_cmd("ps aux | grep 'avahi.*running' | awk 'NR==1{print $13}' | sed 's/\[//' | sed 's/]//'")

check_wifi()

items = [
cpu,
usbdrive, 
"IP: " + ip_address, 
"WiFi Network: " + ssid,
"Host Name: " + host_name,
"Patch: "+ patch,
version, 
]

#print items
