#!/bin/sh
echo "Stopping pd, unmounting USB drive..."
/root/scripts/killpd.sh
oscsend localhost 4001 /oled/aux/clear i 1
oscsend localhost 4001 /oled/aux/line/1 s "Ejecting USB drive..."
umount /usbdrive
echo "done"
oscsend localhost 4001 /oled/aux/line/3 s "Safe to remove."
# set to aux screen
oscsend localhost 4001 /oled/setscreen i 1
oscsend localhost 4001 /reload i 1
