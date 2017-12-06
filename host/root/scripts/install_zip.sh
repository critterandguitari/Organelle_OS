#!/bin/sh

#WORK IN PROGRESS - UNTESTED :) 

#in case the deploy scripts need these
export USER_DIR=${USER_DIR:="/usbdrive"}
export PATCH_DIR=${PATCH_DIR:="/usbdrive/Patches"}
export FW_DIR=${FW_DIR:="/root"}
export SCRIPTS_DIR=$FW_DIR/scripts

oscsend localhost 4001 /oled/setscreen i 1

oscsend localhost 4001 /enableauxsub i 1
oscsend localhost 4001 /oled/aux/clear i 1
oscsend localhost 4001 /oled/aux/line/1 s "installing"
oscsend localhost 4001 /oled/aux/line/2 s "$1"
oscsend localhost 4001 /oled/aux/line/5 s "do not interrupt!"

echo "installing : " $1 
cd $PATCH_DIR 

echo "unzip : " $1 
oscsend localhost 4001 /oled/aux/line/3 s "unzipping"

unzip -o $1 > /tmp/install_files.txt ; ec=$?;
if [ $ec -ne 0 ]
    oscsend localhost 4001 /oled/aux/line/4 s "install FAILED"
    oscsend localhost 4001 /oled/aux/line/5 s "unable to unzip"
    oscsend localhost 4001 /enableauxsub i 0
    exit -1
fi


INSTALL_DIR=`cat /tmp/install_files.txt | head -2 | tail -1 | sed 's/.* \(.*\)\/.*/\1/'`
echo "installed dir : " $INSTALL_DIR

if [ -f $INSTALL_DIR/manifest.txt ]
    oscsend localhost 4001 /oled/aux/line/3 s "checking manifest"
    mv INSTALL_DIR/manifest.txt /tmp
    find $INSTALL_DIR -type f -print0  | xargs -0 sha1sum > manifest.new
    diff manifest.txt manifest.new; ec=$?; 
    if [ $ec -ne 0 ] 
        oscsend localhost 4001 /oled/aux/line/4 s "install FAILED"
        oscsend localhost 4001 /oled/aux/line/5 s "files corrupt"
        oscsend localhost 4001 /enableauxsub i 0
        exit -2
    fi 
fi

if [ -f $INSTALL_DIR/deploy.sh ]
    oscsend localhost 4001 /oled/aux/line/3 s "running deploy"
    $INSTALL_DIR/deploy.sh; ec=$?
    if [ $ec -lt 0 ] 
        oscsend localhost 4001 /enableauxsub i 0
        oscsend localhost 4001 /oled/aux/line/4 s "install FAILED"
        oscsend localhost 4001 /oled/aux/line/5 s "deploy failed"
        oscsend localhost 4001 /enableauxsub i 0
        exit $ec
    fi

    # use exit code from deploy.sh
    oscsend localhost 4001 /enableauxsub i 0
    exit $ec 
fi

#no deploy, so assume success
oscsend localhost 4001 /enableauxsub i 0
exit 0

