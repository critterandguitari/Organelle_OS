#!/bin/sh

USER_DIR=${USER_DIR:="/usbdrive"}

oscsend localhost 4001 /oled/aux/clear i 1
oscsend localhost 4001 /oled/aux/line/2 s "Starting..."
oscsend localhost 4001 /oled/setscreen i 1

python2 "/root/scripts/wifi_setup.py" "$USER_DIR"

