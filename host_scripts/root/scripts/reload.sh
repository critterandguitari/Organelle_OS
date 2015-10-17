#!/bin/sh
/root/scripts/killpd.sh
/root/automount/mount.sh 
oscsend localhost 4001 /reload i 1
