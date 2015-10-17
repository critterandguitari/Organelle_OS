#!/bin/sh
killall pd
oscsend localhost 4001 /oled/line/1 s "Ejecting USB drive..."
echo "unmounting USB drive..."
umount /mnt/usbdrive
sleep 1
oscsend localhost 4001 /oled/line/3 s "Safe to remove."
echo "done"
oscsend localhost 4001 /reload s "Ejecting USB drive."
