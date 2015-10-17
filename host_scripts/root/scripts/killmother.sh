#!/bin/sh
# quit Pd in 3 steps

# give pd a chance to shut itself off
oscsend localhost 4001 /quitmother i 1
sleep .1

# kill pd SIGTERM 
killall mother
sleep .1

# and kill SIGKILL 
killall -s 9 mother

