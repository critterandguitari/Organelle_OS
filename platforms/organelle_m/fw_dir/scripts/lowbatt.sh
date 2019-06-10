#!/bin/sh

# USER_DIR=${USER_DIR:="/usbdrive"}
# PATCH_DIR=${PATCH_DIR:="/usbdrive/Patches"}
FW_DIR=${FW_DIR:="/root"}
SCRIPTS_DIR=$FW_DIR/scripts

oscsend localhost 4001 /oled/aux/clear i 1
oscsend localhost 4001 /oled/aux/line/3 s "Low battery"
oscsend localhost 4001 /oled/setscreen i 1
oscsend localhost 4001 /enableauxsub i 1

$SCRIPTS_DIR/killpatch.sh
$SCRIPTS_DIR/killmother.sh

# shutdown wifi

sudo shutdown -h now
