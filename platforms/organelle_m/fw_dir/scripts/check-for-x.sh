#!/bin/sh

# USER_DIR=${USER_DIR:="/usbdrive"}
# PATCH_DIR=${PATCH_DIR:="/usbdrive/Patches"}
# FW_DIR=${FW_DIR:="/root"}
# SCRIPTS_DIR=$FW_DIR/scripts

if ! xset q > /dev/null; then
    echo "X server not found..."
    exit 0
else
    echo "X server found..."
    exit 1
fi


