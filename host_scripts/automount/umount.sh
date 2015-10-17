#!/bin/sh
#
# USAGE: udev-auto-unmount.sh DEVICE
#   DEVICE   is the actual device node at /dev/DEVICE
# 
# This script takes a device name, looks up the partition label and
# type, creates /media/LABEL and mounts the partition.  Mount options
# are hard-coded below.

DEVICE=$1

# check input
if [ -z "$DEVICE" ]; then
   exit 1
fi

#test that the device is already mounted
MOUNTPT=/mnt/usbdrive
if [ -z "${MOUNTPT}" ]; then
   echo "error: the device is not already mounted"
   exit 1
fi

# test mountpoint - it should exist
if [ -e "${MOUNTPT}" ]; then
#   mkdir /root/automount/unmounting
   # very naive; just run and pray
   umount -l "${MOUNTPT}" && exit 0 

   echo "error: ${MOUNTPT} failed to unmount."
   exit 1
fi

echo "error: ${MOUNTPT} does not exist"
exit 1
