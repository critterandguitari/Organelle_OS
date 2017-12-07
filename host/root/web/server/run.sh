#!/bin/sh

#kill webserver if already running
kill `cat /tmp/webserver.pid`
sleep 1
kill -9 `cat /tmp/webserver.pid`
rm /tmp/webserver.pid



IP=$(ip -o -4 addr list wlan0 | awk '{print $4}' | cut -d/ -f1)
oscsend localhost 4001 /oled/aux/clear i 1

oscsend localhost 4001 /oled/aux/line/1 s "Web Server"
oscsend localhost 4001 /oled/aux/line/2 s "Visit here:"
oscsend localhost 4001 /oled/aux/line/3 s "http://$IP"
oscsend localhost 4001 /oled/aux/line/4 s "with your browser."
oscsend localhost 4001 /oled/aux/line/5 s " "

# set to aux screen, signals screen update
oscsend localhost 4001 /oled/setscreen i 1



# start webserver
cd /root/web/server
python2 server.py & echo $! > /tmp/webserver.pid
