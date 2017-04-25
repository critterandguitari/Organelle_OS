#!/bin/bash

echo "Updating OS"

# remount root read write
/root/scripts/remount-rw.sh

# copy files
cp -f root/mother.pd /root
cp -f root/mother /root
cp root/.bash_profile /root
cp root/.jwmrc /root
cp root/.pdsettings /root
cp root/scripts/* /root/scripts

# sync
sync 

# normally we'd want to remount read only, but this is not possible because of cp -f
