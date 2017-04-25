#!/bin/bash

echo "about to save new patch..."

# remove old state directory
rm -fr /tmp/state
mkdir /tmp/state

oscsend localhost 4001 /oled/aux/line/1 s "Saving New"
oscsend localhost 4000 /saveStateNew i 1

# allow patch to save stuff in state folder if it wants
sleep .25 

# get newest name
OLDNAME=$( ls /tmp/curpatchname )
N=1
NEWNAME="${OLDNAME} ${N}"
while [[ -d "/usbdrive/Patches/${NEWNAME}" ]] ; do
    N=$(($N+1))
    NEWNAME="${OLDNAME} ${N}"
done

echo $NEWNAME

# copy current patch to a new one
cp -Hr /tmp/patch/ "/usbdrive/Patches/${NEWNAME}"

# remove previous state
rm -fr "/usbdrive/Patches/${NEWNAME}/state"

# copy knobs.txt and any other states saved by the patch
cp -Hr /tmp/state/ "/usbdrive/Patches/${NEWNAME}/state"

# set to aux screen, signals screen update
oscsend localhost 4001 /oled/setscreen i 1

sync

# reload
oscsend localhost 4001 /reload i 1
oscsend localhost 4001 /gohome i 1
oscsend localhost 4001 /loadPatch s "${NEWNAME}"


