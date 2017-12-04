#!/bin/sh

# export PATCH_DIR=${PATCH_DIR:="/usbdrive/Patches"}
export FW_DIR=${FW_DIR:="/root"}
export SCRIPTS_DIR=$FW_DIR/scripts

clear
$SCRIPTS_DIR/killmother.sh
$SCRIPTS_DIR/setup.sh
#$SCRIPTS_DIR/check-for-usb-drive.sh
$SCRIPTS_DIR/mount.sh 

mkdir -p /tmp/pids

if grep -qs "/usbdrive" /proc/mounts 
then
    echo using usbdrive for user dir
    export USER_DIR=/usbdrive
else 
    echo no usbdrive found, try sdcard
    if grep -qs "/sdcard" /proc/mounts 
    then 
        echo using sdcard for user dir
        export USER_DIR=/sdcard
    else
        echo no sdcard found, use usbdrive
        # not practical, as its not mounted, but we have no choice
        export USER_DIR=/usbdrive
    fi
fi


M_DIR=/root
FW_DIR=/root
# mother usbdrive/system -> usbdrive/Firmware -> sdcard/Firmrware -> root
if [ -f /usbdrive/System/mother ]
then
    # for backwards compatibility (<2.2)
    M_DIR=/usbdrive/System

elif [ -f /usbdrive/Firmware/mother ]
then 
    M_DIR=/usbdrive/Firmware
    if [ -d /usbdrive/Firmware/scripts ]
    then
        export FW_DIR="/usbdrive/Firmware"
    fi
elif [ -f /sdcard/Firmware/mother ]
then 
    M_DIR=/sdcard/Firmware
    if [ -d /sdcard/Firmware/scripts ]
    then
        export FW_DIR="/sdcard/Firmware"
    fi
fi
echo running $M_DIR/mother with scripts $FW_DIR/scripts
$M_DIR/mother &
