#!/bin/bash

grep -q 'ID=archarm' /etc/os-release; 
if [ ! $? -eq 0 ] 
then 
	export FW_DIR=${FW_DIR:="~/fw_dir"}
else 
	export FW_DIR=${FW_DIR:="/root"}
fi


USER_DIR=${USER_DIR:="/usbdrive"}
SCRIPTS_DIR=$FW_DIR/scripts
WORK_DIR=${WORK_DIR:="${USER_DIR}"}

cd $WORK_DIR

# clear aux screen
oscsend localhost 4001 /oled/aux/clear i 1
oscsend localhost 4001 /oled/setscreen i 1
oscsend localhost 4001 /oled/aux/line/2 s "stopping"
oscsend localhost 4001 /oled/aux/line/3 s "VNC"

$SCRIPTS_DIR/stop-gui.sh
