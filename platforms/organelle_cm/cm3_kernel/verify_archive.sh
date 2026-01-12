#!/bin/bash

ARCHIVE="$1"
if [ -z "$ARCHIVE" ]; then
    echo "Usage: $0 <archive.tar.gz>"
    exit 1
fi

if [ ! -f "$ARCHIVE" ]; then
    echo "Error: Archive '$ARCHIVE' not found"
    exit 1
fi

# Create temporary directory
TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

echo "Extracting archive to temporary directory..."
tar -xzf "$ARCHIVE" -C "$TMPDIR"

echo "Comparing files..."
echo "===================="

cd "$TMPDIR"

# If everything is in a subdirectory, cd into it
# (handles case where archive has a top-level directory)
SUBDIRS=$(find . -maxdepth 1 -type d ! -name ".")
SUBDIR_COUNT=$(echo "$SUBDIRS" | wc -l)

if [ $SUBDIR_COUNT -eq 1 ] && [ -n "$SUBDIRS" ]; then
    echo "Archive has top-level directory, entering it..."
    cd "$SUBDIRS"
fi

# Initialize counters
MATCH_COUNT=0
DIFFERENT_COUNT=0
MISSING_COUNT=0
TYPE_MISMATCH_COUNT=0

# Find all files (not directories)
find . -type f | while read -r file; do
    # Remove leading ./
    filepath="${file#./}"
    system_file="/$filepath"
    
    if [ ! -e "$system_file" ]; then
        echo "MISSING: $system_file (exists in archive but not on system)"
        ((MISSING_COUNT++))
    elif [ ! -f "$system_file" ]; then
        echo "TYPE_MISMATCH: $system_file (not a regular file on system)"
        ((TYPE_MISMATCH_COUNT++))
    else
        # Compare using sha256sum
        archive_hash=$(sha256sum "$file" | awk '{print $1}')
        system_hash=$(sha256sum "$system_file" 2>/dev/null | awk '{print $1}')
        
        if [ "$archive_hash" != "$system_hash" ]; then
            echo "DIFFERENT: $system_file"
            echo "  Archive hash: $archive_hash"
            echo "  System hash:  $system_hash"
            ((DIFFERENT_COUNT++))
        else
            echo "MATCH: $system_file"
            ((MATCH_COUNT++))
        fi
    fi
    
    # Write counters to temp file (since this runs in a subshell)
    echo "$MATCH_COUNT $DIFFERENT_COUNT $MISSING_COUNT $TYPE_MISMATCH_COUNT" > "$TMPDIR/.counts"
done

# Read final counts
if [ -f "$TMPDIR/.counts" ]; then
    read MATCH_COUNT DIFFERENT_COUNT MISSING_COUNT TYPE_MISMATCH_COUNT < "$TMPDIR/.counts"
fi

echo "===================="
echo "SUMMARY REPORT"
echo "===================="
echo "Files matched:        $MATCH_COUNT"
echo "Files different:      $DIFFERENT_COUNT"
echo "Files missing:        $MISSING_COUNT"
echo "Type mismatches:      $TYPE_MISMATCH_COUNT"
echo "--------------------"
TOTAL=$((MATCH_COUNT + DIFFERENT_COUNT + MISSING_COUNT + TYPE_MISMATCH_COUNT))
echo "Total files checked:  $TOTAL"
echo "===================="