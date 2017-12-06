#!/bin/sh
echo create install zip for $1

INSTALL_DIR=$1

if [ -f $INSTALL_DIR/manifest.txt ]
    echo move existing manifest away
    mv INSTALL_DIR/manifest.txt /tmp
fi
find $INSTALL_DIR -type f -print0  | xargs -0 sha1sum > $INSTALL_DIR/manifest.txt

zip -r INSTALL_DIR.zip $INSTALL_DIR/*
