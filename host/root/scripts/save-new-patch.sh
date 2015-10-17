#!/bin/bash

# create tmp state directory if doesn't exist
if [ ! -d "/tmp/state" ]; then
    mkdir /tmp/state
fi

oscsend localhost 4001 /oled/aux/line/1 s "Saving...Reloading..."
oscsend localhost 4000 /saveStateNew i 1

# allow patch to save stuff in state folder if it wants
sleep .25 

N=2
while [[ -d "/usbdrive/Patches/0000$N" ]] ; do
    N=$(($N+1))
done

# copy current patch to a new one
cp -Hr /tmp/patch/ "/usbdrive/Patches/0000$N"

# remove previous state
rm -fr "/usbdrive/Patches/0000$N/state"

# copy knobs.txt and any other states
cp -Hr /tmp/state/ "/usbdrive/Patches/0000$N/state"

# set to aux screen, signals screen update
oscsend localhost 4001 /oled/setscreen i 1

sleep 1

# reload
oscsend localhost 4001 /reload i 1
oscsend localhost 4001 /gohome i 1


