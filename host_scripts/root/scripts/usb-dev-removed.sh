#!/bin/sh
oscsend localhost 4001 /oled/aux/clear i 1
oscsend localhost 4001 /oled/aux/line/1 s "USB Devive Removed!"
oscsend localhost 4001 /oled/aux/line/3 s "Stopping Patch."

# set to aux screen
oscsend localhost 4001 /oled/setscreen i 1
/root/scripts/killpd.sh
oscsend localhost 4001 /reload i 1
