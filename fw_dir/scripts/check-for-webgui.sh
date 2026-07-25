#!/bin/sh

if systemctl is-active --quiet xpra-pd; then
    echo "Patch Editor active..."
    exit 1
else
    echo "Patch Editor not active..."
    exit 0
fi
