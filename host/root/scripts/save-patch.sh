#!/bin/sh

oscsend localhost 4001 /oled/aux/line/1 s "Saving..."

# create state directory if doesn't exist
if [ ! -d "/tmp/patch/state" ]; then
    mkdir /tmp/patch/state
fi

oscsend localhost 4000 /saveState i 1

# set to aux screen, signals screen update
oscsend localhost 4001 /oled/setscreen i 1

sleep 1

oscsend localhost 4001 /oled/setscreen i 3
