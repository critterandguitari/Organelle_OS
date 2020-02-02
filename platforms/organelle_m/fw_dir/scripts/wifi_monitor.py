import imp
import time
import os

fw_dir = os.getenv("FW_DIR")
wifi = imp.load_source('wifi_control', fw_dir + '/scripts/wifi_control.py')


while True:
    wifi.initialize_state()
    if wifi.wifi_connected() :
        os.system('oscsend localhost 4001 /wifiStatus i 1')
    else :
        os.system('oscsend localhost 4001 /wifiStatus i 0')
    time.sleep(2)

