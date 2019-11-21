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

items = {
    "ssid" : ["WiFi Network", "not connected"],
    "ip_address" : ["IP Address", "not connected"],
    "usbdrive" : ["USB Drive", ""],
    "version" : ["OS Version", ""],
    "patch" : ["Current Patch", ""],
    "host_name" : ["Hostname", ""]
}

def check_wifi():
    global items
    wifi.initialize_state()
    if wifi.wifi_connected() :
        items["ssid"][1] = wifi.current_net
        items["ip_address"][1] = wifi.ip_address

# returns output if exit code 0, NA otherwise
def run_cmd(cmd) :
    ret = 'None'
    try:
        ret = subprocess.check_output(['bash', '-c', cmd], close_fds=True)
    except: pass
    return ret

# get info
def get_info() :
    global items

    items["usbdrive"][1] = run_cmd("grep usbdrive /proc/mounts | awk '{print $1}' | sed -e 's/\/dev\///'")
    version = run_cmd("cat " + fw_dir + "/version")
    build_tag = run_cmd("cat " + fw_dir + "/buildtag")
    items["version"][1] = version + build_tag
    items["patch"][1] = run_cmd("ls /tmp/curpatchname")
    items["host_name"][1] = run_cmd("ps aux | grep 'avahi.*running' | awk 'NR==1{print $13}' | sed 's/\[//' | sed 's/]//'")
    check_wifi()

