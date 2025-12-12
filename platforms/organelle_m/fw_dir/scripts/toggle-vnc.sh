#!/bin/bash

export FW_DIR=${FW_DIR:="$HOME/fw_dir"}

# Check if vncserver-virtual is running
if pgrep -x "Xvnc" > /dev/null || pgrep -f "vncserver-virtual" > /dev/null; then
    echo "VNC server is running. Stopping it..." | systemd-cat --identifier=Organelle
    
    oscsend localhost 4001 /oled/gClear ii 4 1
    oscsend localhost 4001 /oled/gPrintln iiiiis 4 10 10 8 1 "Stopping VNC"
    oscsend localhost 4001 /oled/gFlip ii 4 1
    oscsend localhost 4001 /oled/setscreen i 4
    
    pkill -f "vncserver-virtual"
    pkill -x "Xvnc"
    vncserver-virtual -kill :1 2>&1 | systemd-cat --identifier=Organelle
    echo "VNC server stopped." | systemd-cat --identifier=Organelle

    oscsend localhost 4001 /oled/gClear ii 4 1
    oscsend localhost 4001 /oled/gPrintln iiiiis 4 10 10 8 1 "Stopped VNC"
    oscsend localhost 4001 /oled/gFlip ii 4 1
    oscsend localhost 4001 /oled/setscreen i 4
    sudo mount -o remount,ro /
    $SCRIPTS_DIR/start-mother.sh 
else
    echo "VNC server is not running. Starting it..." | systemd-cat --identifier=Organelle
    
    oscsend localhost 4001 /oled/gClear ii 4 1
    oscsend localhost 4001 /oled/gPrintln iiiiis 4 10 10 8 1 "Starting VNC"
    oscsend localhost 4001 /oled/gFlip ii 4 1
    oscsend localhost 4001 /oled/setscreen i 4
    
    sudo mount -o remount,rw /
    vncserver-virtual -geometry 1920x1080 2>&1 | systemd-cat --identifier=Organelle
    echo "VNC server started." | systemd-cat --identifier=Organelle
    
    oscsend localhost 4001 /oled/gClear ii 4 1
    oscsend localhost 4001 /oled/gPrintln iiiiis 4 10 10 8 1 "Started VNC"
    oscsend localhost 4001 /oled/gFlip ii 4 1
    oscsend localhost 4001 /oled/setscreen i 4
fi