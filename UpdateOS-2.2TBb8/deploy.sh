#!/bin/bash


# USER_DIR=${USER_DIR:="/usbdrive"}
# PATCH_DIR=${PATCH_DIR:="/usbdrive/Patches"}
FW_DIR=${FW_DIR:="/root"}
SCRIPTS_DIR=$FW_DIR/scripts


echo "Updating OS to " `cat /usbdrive/Patches/UpdateOS-2.2TBb8/version` `cat /usbdrive/Patches/UpdateOS-2.2TBb8/buildtag`

cd /usbdrive/Patches/UpdateOS-2.2TBb8
mv files.sha1 /tmp
openssl sha1 `cat manifest` > /tmp/files.tmp

if ! cmp /tmp/files.sha1 /tmp/files.tmp 
then
mv /tmp/files.sha1 .
oscsend localhost 4001 /oled/line/2 s "Upgrade failed:sha1"
sleep 1
#kill this upgrade patch
$SCRIPTS_DIR/killpd.sh

exit -1
fi

mv /tmp/files.sha1 . 
# remount root read write
/root/scripts/remount-rw.sh

# copy files
cp -f /usbdrive/Patches/UpdateOS-2.2TBb8/mother.pd /root
cp -f /usbdrive/Patches/UpdateOS-2.2TBb8/mother /root
cp -f /usbdrive/Patches/UpdateOS-2.2TBb8/.bash_profile /root
cp -f /usbdrive/Patches/UpdateOS-2.2TBb8/.jwmrc /root
cp -f /usbdrive/Patches/UpdateOS-2.2TBb8/.pdsettings /root
cp -f /usbdrive/Patches/UpdateOS-2.2TBb8/version /root
cp -f /usbdrive/Patches/UpdateOS-2.2TBb8/buildtag /root
cp -f /usbdrive/Patches/UpdateOS-2.2TBb8/scripts/* /root/scripts

# sync
sync 

# just chill
sleep 1


$SCRIPTS_DIR/shutdown.sh
