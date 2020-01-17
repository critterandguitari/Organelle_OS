#!/bin/bash
killall wpa_supplicant
killall dhcpcd

USER_DIR=$(cat /tmp/user_dir)

AP_FILE="${USER_DIR}/ap.txt"

if [ -f "$AP_FILE" ]; then
    echo "$AP_FILE exists"
    NET=$(head -n 1 $AP_FILE)
    PW=$(tail -n 1 $AP_FILE)
else 
    echo "$AP_FILE does not exist, using default"
    NET=Organelle
    PW=coolmusic
fi

/root/scripts/create_ap --no-virt -n wlan0 $NET $PW

