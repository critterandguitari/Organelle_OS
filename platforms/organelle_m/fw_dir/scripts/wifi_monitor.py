import imp
import time
import os

fw_dir = os.getenv("FW_DIR", "/root")
wifi = imp.load_source('wifi_control', fw_dir + '/scripts/wifi_control.py')

connected = False

while True:
    if wifi.wifi_connected() :
        tmp = True
    else :
        tmp = False
    if tmp != connected :
        connected = tmp
        if connected :
            print "connected"
            os.system('oscsend localhost 4001 /wifiStatus i 1')
        else :
            print "not connected"
            os.system('oscsend localhost 4001 /wifiStatus i 0')
    time.sleep(1)

