#!/bin/sh


# $HOME not set when called from systemd service, so need to use /root
export FW_DIR=${FW_DIR:="/root/fw_dir"}
export USER_DIR=`$FW_DIR/scripts/get-user-dir.sh`
echo using USER_DIR: $USER_DIR


# start webserver
cd $FW_DIR/web/server
python2 server.py 
