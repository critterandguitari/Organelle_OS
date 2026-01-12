#!/bin/bash

ARCHIVE="$1"
MANIFEST="$2"

if [ -z "$ARCHIVE" ] || [ -z "$MANIFEST" ]; then
    echo "Usage: $0 <archive.tar.gz> <manifest.txt>"
    echo ""
    echo "This script will:"
    echo "  1. Extract the archive"
    echo "  2. Copy all files to their system locations (requires root)"
    echo "  3. Verify each file against the manifest"
    exit 1
fi

if [ ! -f "$ARCHIVE" ]; then
    echo "Error: Archive '$ARCHIVE' not found"
    exit 1
fi

if [ ! -f "$MANIFEST" ]; then
    echo "Error: Manifest '$MANIFEST' not found"
    exit 1
fi

# Convert to absolute paths before we change directories
ARCHIVE="$(readlink -f "$ARCHIVE")"
MANIFEST="$(readlink -f "$MANIFEST")"

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root (use sudo)"
    exit 1
fi

# Create temporary directory
TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

echo "========================================"
echo "INSTALLATION SCRIPT"
echo "========================================"
echo "Archive:  $ARCHIVE"
echo "Manifest: $MANIFEST"
echo ""

read -p "This will copy files to system locations. Continue? (yes/no): " CONFIRM
if [ "$CONFIRM" != "yes" ]; then
    echo "Installation cancelled."
    exit 0
fi

echo ""
echo "Extracting archive to temporary directory..."
tar -xzf "$ARCHIVE" -C "$TMPDIR"

cd "$TMPDIR"

# If everything is in a subdirectory, cd into it
SUBDIRS=$(find . -maxdepth 1 -type d ! -name ".")
SUBDIR_COUNT=$(echo "$SUBDIRS" | wc -l)

if [ $SUBDIR_COUNT -eq 1 ] && [ -n "$SUBDIRS" ]; then
    echo "Archive has top-level directory, entering it..."
    cd "$SUBDIRS"
fi

echo ""
echo "========================================"
echo "COPYING FILES"
echo "========================================"

COPY_SUCCESS=0
COPY_FAILED=0

# Read manifest and copy files
while IFS= read -r line; do
    # Parse manifest line: "hash  /path/to/file"
    hash=$(echo "$line" | awk '{print $1}')
    filepath=$(echo "$line" | awk '{print $2}')
    
    # Remove leading / to get relative path
    relpath="${filepath#/}"
    
    if [ ! -f "$relpath" ]; then
        echo "WARNING: File not in archive: $filepath"
        ((COPY_FAILED++))
        continue
    fi
    
    # Create parent directory if needed
    parent_dir=$(dirname "$filepath")
    mkdir -p "$parent_dir"
    
    # Copy file (will be owned by root since running with sudo)
    if cp "$relpath" "$filepath" 2>/tmp/cp_error.txt; then
        echo "COPIED: $filepath"
        ((COPY_SUCCESS++))
    else
        ERROR_MSG=$(cat /tmp/cp_error.txt 2>/dev/null)
        echo "FAILED: $filepath"
        echo "  Error: $ERROR_MSG"
        ((COPY_FAILED++))
    fi
done < "$MANIFEST"

echo ""
echo "========================================"
echo "VERIFYING FILES"
echo "========================================"

VERIFY_SUCCESS=0
VERIFY_FAILED=0

# Verify all files against manifest
while IFS= read -r line; do
    expected_hash=$(echo "$line" | awk '{print $1}')
    filepath=$(echo "$line" | awk '{print $2}')
    
    if [ ! -f "$filepath" ]; then
        echo "MISSING: $filepath"
        ((VERIFY_FAILED++))
        continue
    fi
    
    actual_hash=$(sha256sum "$filepath" 2>/dev/null | awk '{print $1}')
    
    if [ "$expected_hash" = "$actual_hash" ]; then
        echo "VERIFIED: $filepath"
        ((VERIFY_SUCCESS++))
    else
        echo "HASH MISMATCH: $filepath"
        echo "  Expected: $expected_hash"
        echo "  Actual:   $actual_hash"
        ((VERIFY_FAILED++))
    fi
done < "$MANIFEST"

echo ""
echo "========================================"
echo "INSTALLATION SUMMARY"
echo "========================================"
echo "Copy successful:       $COPY_SUCCESS"
echo "Copy failed:           $COPY_FAILED"
echo "Verify successful:     $VERIFY_SUCCESS"
echo "Verify failed:         $VERIFY_FAILED"
echo "========================================"

if [ $COPY_FAILED -gt 0 ] || [ $VERIFY_FAILED -gt 0 ]; then
    echo "WARNING: Some files failed to copy or verify!"
    exit 1
else
    echo "Installation completed successfully!"
    exit 0
fi
