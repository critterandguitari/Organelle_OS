#!/bin/sh
killall pd
/root/automount/mount.sh sda1
oscsend localhost 4001 /reload i 1
