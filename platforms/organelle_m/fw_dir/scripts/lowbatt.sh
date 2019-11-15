#!/bin/sh

# USER_DIR=${USER_DIR:="/usbdrive"}
# PATCH_DIR=${PATCH_DIR:="/usbdrive/Patches"}
FW_DIR=${FW_DIR:="/home/music/fw_dir"}
SCRIPTS_DIR=$FW_DIR/scripts

oscsend localhost 4001 /oled/gClear ii 4 1
oscsend localhost 4001 /oled/gPrintln iiiiis 4 10 10 8 1 "Low Battery"
oscsend localhost 4001 /oled/gPrintln iiiiis 4 10 30 8 1 "Auto Shutdown"
oscsend localhost 4001 /oled/gFlip ii 4 1
oscsend localhost 4001 /oled/setscreen i 4

$SCRIPTS_DIR/killpatch.sh
$SCRIPTS_DIR/killmother.sh

# shutdown wifi
sudo systemctl stop createap
sudo killall dhcpcd
sudo killall wpa_supplicant

sudo shutdown -h now
