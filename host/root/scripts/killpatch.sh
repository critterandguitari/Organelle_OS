#!/bin/sh

# USER_DIR=${USER_DIR:="/usbdrive"}
# PATCH_DIR=${PATCH_DIR:="/usbdrive/Patches"}
# FW_DIR=${FW_DIR:="/root"}
# SCRIPTS_DIR=$FW_DIR/scripts

# quiting patches
# attempt to do 'kindly first', then get more brutal 
# we do not if its pd, or sc or run script , try each

# quit Pd in 3 steps

# give pd a chance to shut itself off
oscsend localhost 4000 /quitpd i 1
sleep .12

# kill pd SIGTERM 
killall pd
sleep .1

# and kill SIGKILL 
killall -s 9 pd

#kill all pids with SIGTERM
kill `cat /tmp/pids/*.pid`


#kill all pids with SIGKILL
kill -9 `cat /tmp/pids/*.pid`


rm /tmp/pids/*.pid

# turn off led, just to be sure
oscsend localhost 4001 /led i 0

# clean up
# remove old state directory
rm -fr /tmp/state
mkdir /tmp/state


