#!/bin/bash

echo "Updating OS"

# remount root read write
/root/scripts/remount-rw.sh

# copy files
cp -f root/mother.pd /root
cp -f root/mother /root
cp -f root/.bash_profile /root
cp -f root/.jwmrc /root
cp -f root/.pdsettings /root
cp -f root/scripts/* /root/scripts
cp -f root/version /root/version

# sync
sync 

# normally we'd want to remount read only, but this is not possible because of cp -f
