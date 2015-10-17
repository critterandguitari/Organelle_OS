#!/bin/sh
/root/scripts/killpd.sh
oscsend localhost 4001 /oled/line/1 s "Ejecting USB drive..."
echo "unmounting USB drive..."
umount /usbdrive
sleep 1
oscsend localhost 4001 /oled/line/3 s "Safe to remove."
echo "done"
oscsend localhost 4001 /reload i 1
