#!/bin/sh
oscsend localhost 4001 /shutdown
sleep .5
shutdown -h now
#echo "shjutting down"
