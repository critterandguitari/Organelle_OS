#!/bin/bash

# Script to resize partition 3 by 20MB and zero out free space
# Usage: ./resize_image.sh your_image.img

set -e # Exit on any error

if [ $# -ne 1 ]; then
echo "Usage: $0 <image_file.img>"
exit 1
fi

IMAGE_FILE="$1"

if [ ! -f "$IMAGE_FILE" ]; then
echo "Error: Image file '$IMAGE_FILE' not found"
exit 1
fi

echo "Processing image: $IMAGE_FILE"

# Step 1: Set up loop device
echo "Step 1: Setting up loop device..."
LOOP_DEVICE=$(sudo losetup -f)
echo "Using loop device: $LOOP_DEVICE"
sudo losetup -P "$LOOP_DEVICE" "$IMAGE_FILE"

# Verify partitions were created
sleep 1
if [ ! -e "${LOOP_DEVICE}p3" ]; then
echo "Error: Partition ${LOOP_DEVICE}p3 not found"
sudo losetup -d "$LOOP_DEVICE"
exit 1
fi

# Step 2: Show current partition layout
echo "Step 2: Current partition layout:"
sudo fdisk -l "$LOOP_DEVICE"

# Step 3: Check filesystem on p3
echo "Step 3: Checking filesystem on partition 3..."
sudo e2fsck -f "${LOOP_DEVICE}p3"

# Step 4: Resize filesystem on p3 (shrink by 20MB)
echo "Step 4: Resizing filesystem on partition 3..."
# Calculate new size: current blocks - (20MB in 4K blocks)
# 20MB = 20971520 bytes, 20971520/4096 = 5120 blocks to subtract
CURRENT_BLOCKS=$(sudo tune2fs -l "${LOOP_DEVICE}p3" | grep "Block count:" | awk '{print $3}')
NEW_BLOCKS=$((CURRENT_BLOCKS - 5120))
echo "Resizing from $CURRENT_BLOCKS to $NEW_BLOCKS blocks"
sudo resize2fs "${LOOP_DEVICE}p3" "$NEW_BLOCKS"

# Step 5: Zero out free space on p2 and p3
echo "Step 5: Zeroing out free space..."
sudo mkdir -p /mnt/loop2 /mnt/loop3

echo " Zeroing p2..."
sudo mount "${LOOP_DEVICE}p2" /mnt/loop2
sudo dd if=/dev/zero of=/mnt/loop2/zerofill bs=1M 2>/dev/null || true
sudo rm -f /mnt/loop2/zerofill
sudo umount /mnt/loop2

echo " Zeroing p3..."
sudo mount "${LOOP_DEVICE}p3" /mnt/loop3
sudo dd if=/dev/zero of=/mnt/loop3/zerofill bs=1M 2>/dev/null || true
sudo rm -f /mnt/loop3/zerofill
sudo umount /mnt/loop3

sudo rmdir /mnt/loop2 /mnt/loop3

# Step 6: Calculate new partition end sector
echo "Step 6: Calculating new partition boundaries..."
# Get current partition info
PART_INFO=$(sudo fdisk -l "$LOOP_DEVICE" | grep "${LOOP_DEVICE}p3")
START_SECTOR=$(echo "$PART_INFO" | awk '{print $2}')
CURRENT_END=$(echo "$PART_INFO" | awk '{print $3}')
# Subtract 40960 sectors (20MB = 20*1024*1024/512)
NEW_END=$((CURRENT_END - 40960))

echo " Partition 3 start: $START_SECTOR"
echo " Current end: $CURRENT_END"
echo " New end: $NEW_END"

# Step 7: Resize partition with fdisk
echo "Step 7: Resizing partition 3..."
sudo fdisk "$LOOP_DEVICE" << EOF
d
3
n
p
3
$START_SECTOR
$NEW_END
w
EOF

# Step 8: Final filesystem check
echo "Step 8: Final filesystem check..."
sudo e2fsck -f "${LOOP_DEVICE}p3"

# Step 9: Show final partition layout
echo "Step 9: Final partition layout:"
sudo fdisk -l "$LOOP_DEVICE"

# Step 10: Clean up loop device
echo "Step 10: Cleaning up..."
sudo losetup -d "$LOOP_DEVICE"

# Step 11: Truncate image file to target size
TARGET_SIZE=7717519360
CURRENT_SIZE=$(stat -c%s "$IMAGE_FILE")
echo "Step 11: Truncating image file..."
echo " Current size: $CURRENT_SIZE bytes"
echo " Target size: $TARGET_SIZE bytes"

if [ "$CURRENT_SIZE" -gt "$TARGET_SIZE" ]; then
truncate -s "$TARGET_SIZE" "$IMAGE_FILE"
NEW_SIZE=$(stat -c%s "$IMAGE_FILE")
echo " New size: $NEW_SIZE bytes"
echo " Truncated $(($CURRENT_SIZE - $NEW_SIZE)) bytes"
else
echo " Image is already smaller than target size, no truncation needed"
fi

echo "Done! Image has been resized successfully."
echo "Partition 3 has been shrunk by 20MB, free space zeroed, and image truncated."
