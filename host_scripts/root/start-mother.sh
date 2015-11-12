#!/bin/sh
killall mother
killall pd
/root/setup.sh
/root/check-for-usb-drive.sh
/root/mother &
