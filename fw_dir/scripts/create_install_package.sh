#!/bin/sh
if [ -z "$1" ]; then
    echo "Usage: create_install_package.sh <install_dir>"
    exit 1
fi
echo create install package for $1

INSTALL_DIR=$1

if [ -f "$INSTALL_DIR/deploy.sh" ]
then
    chmod +x "$INSTALL_DIR/deploy.sh"
fi

if [ -f "$INSTALL_DIR/manifest.txt" ]
then
    echo move existing manifest away
    mv "$INSTALL_DIR/manifest.txt" /tmp
fi
rm -rf __MACOSX

find "$INSTALL_DIR" -type f ! -name "._*" -print0  | xargs -0 sha1sum > /tmp/manifest.new
mv /tmp/manifest.new "$INSTALL_DIR/manifest.txt"

zip -r "$INSTALL_DIR.zop" "$INSTALL_DIR"/*
