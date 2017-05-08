#!/bin/bash


# encoder wheel is ignored (until /gohome is called at end of script)
oscsend localhost 4001 /enableauxsub i 1

# clear aux screen
oscsend localhost 4001 /oled/aux/clear i 1
oscsend localhost 4001 /oled/setscreen i 1


/root/scripts/oscdump2 4002 | /root/scripts/set-midi-ch.sh

echo "cool set the ch"

#oscsend localhost 4001 /gohome i 1
oscsend localhost 4001 /oled/setscreen i 2


