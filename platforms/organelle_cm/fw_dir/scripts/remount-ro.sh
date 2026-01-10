#!/bin/sh

# USER_DIR=${USER_DIR:="/usbdrive"}
# PATCH_DIR=${PATCH_DIR:="/usbdrive/Patches"}
# FW_DIR=${FW_DIR:="$HOME/fw_dir"}
# SCRIPTS_DIR=$FW_DIR/scripts

mount / -o remount,ro
mount /boot/firmware -o remount,ro
