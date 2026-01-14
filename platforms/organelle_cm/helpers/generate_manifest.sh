#!/bin/bash

ARCHIVE="$1"
OUTPUT="${2:-manifest.txt}"

if [ -z "$ARCHIVE" ]; then
    echo "Usage: $0 <archive.tar.gz> [output_manifest.txt]"
    exit 1
fi

if [ ! -f "$ARCHIVE" ]; then
    echo "Error: Archive '$ARCHIVE' not found"
    exit 1
fi

# Save the original working directory
ORIG_DIR="$(pwd)"

# Create temporary directory
TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

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

echo "Generating manifest..."
> "$OUTPUT.tmp"

FILE_COUNT=0
# Find all files and generate hashes
find . -type f | sort | while read -r file; do
    filepath="${file#./}"
    hash=$(sha256sum "$file" | awk '{print $1}')
    echo "$hash  /$filepath" >> "$OUTPUT.tmp"
    ((FILE_COUNT++))
    if [ $((FILE_COUNT % 100)) -eq 0 ]; then
        echo "  Processed $FILE_COUNT files..."
    fi
done

# Move manifest to original location
mv "$OUTPUT.tmp" "$ORIG_DIR/$OUTPUT"

TOTAL=$(wc -l < "$ORIG_DIR/$OUTPUT")
echo "Manifest generated: $ORIG_DIR/$OUTPUT"
echo "Total files: $TOTAL"