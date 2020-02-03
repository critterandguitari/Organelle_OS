#!/bin/sh


export FW_DIR=${FW_DIR:="$HOME/fw_dir"}
export USER_DIR=`$FW_DIR/scripts/get-user-dir.sh`
echo using USER_DIR: $USER_DIR


# start webserver
cd $FW_DIR/web/server
python2 server.py 
