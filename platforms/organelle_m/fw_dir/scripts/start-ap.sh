#!/bin/bash
killall wpa_supplicant
killall dhcpcd
/home/music/fw_dir/scripts/create_ap --no-virt -n wlan0 Organelle coolmusic
