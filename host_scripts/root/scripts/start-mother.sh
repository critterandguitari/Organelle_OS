#!/bin/sh
killall mother
killall pd
/root/scripts/setup.sh
clear
#/root/scripts/check-for-usb-drive.sh
/root/scripts/mount.sh 
# check if mother is present in /mnt/usbdrive/System
if [ -f /mnt/usbdrive/System/mother ]; then
    echo "running mother from /mnt/usbdrive/System"
    /mnt/usbdrive/System/mother &
else
    echo "running mother from /root/"
    /root/mother &
fi

