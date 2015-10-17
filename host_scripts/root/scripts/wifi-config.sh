#!/bin/sh
ip link set wlan0 up
#wpa_supplicant -D nl80211,wext -i wlan0 -c <(wpa_passphrase "birds2" "rollieopus") &
wpa_supplicant -D nl80211,wext -i wlan0 -c <(wpa_passphrase "CBCI-AD15-2.4" "AD3PAX7WFHE3D94K") &

