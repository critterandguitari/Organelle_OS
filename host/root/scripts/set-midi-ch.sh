#!/bin/bash

CH=1

while read line; do
#    echo $line
        
    if [ "$line" == "/encoder/turn i 0" ]
    then
        CH=$(($CH-1))
        if (( $CH < 0 )); then
            CH=0
        fi
        echo $CH
        oscsend localhost 4001 /oled/aux/line/2 s "MIDI CH: $CH"
        exit
    fi
    
    if [ "$line" == "/encoder/turn i 1" ]
    then 
        CH=$(($CH+1))
        if (( $CH > 16 )); then
            CH=16
        fi
        echo $CH
        oscsend localhost 4001 /oled/aux/line/2 s "MIDI CH: $CH"
    fi

done


