#!/bin/bash

set -e

# Konfiguration – bitte bei Bedarf anpassen
WORK_DIR="/home/leon/Documents/yocto-docker/shared_output"
YOCTO_IMG_DIR="$WORK_DIR/stm32mp13-disco"

# Dateinamen der einzelnen Partitionen
BOOTFS_IMG="st-image-bootfs-openstlinux-weston-stm32mp13-disco.ext4"
ROOTFS_IMG="st-image-weston-openstlinux-weston-stm32mp13-disco.ext4"

# Ergebnis-Image
OUTPUT_IMG="$WORK_DIR/stm32mp13-weston.img"

# Größe der einzelnen Partitionen in MB (geschätzt)
BOOTFS_SIZE_MB=64
ROOTFS_SIZE_MB=512

# Imagegröße berechnen (+ etwas Puffer)
IMG_SIZE_MB=$((BOOTFS_SIZE_MB + ROOTFS_SIZE_MB + 512))

echo "Erstelle leeres Disk-Image mit $IMG_SIZE_MB MB..."
dd if=/dev/zero of="$OUTPUT_IMG" bs=1M count="$IMG_SIZE_MB"

echo "Partitioniere das Image..."
parted "$OUTPUT_IMG" --script -- mklabel msdos
parted "$OUTPUT_IMG" --script -- mkpart primary ext4 4MiB "$((4 + BOOTFS_SIZE_MB))"MiB
parted "$OUTPUT_IMG" --script -- mkpart primary ext4 "$((4 + BOOTFS_SIZE_MB))"MiB 100%

echo "Binde Image an Loop-Device..."
LOOP_DEV=$(losetup --show -fP "$OUTPUT_IMG")

echo "Kopiere Bootfs..."
dd if="$YOCTO_IMG_DIR/$BOOTFS_IMG" of="${LOOP_DEV}p1" bs=1M status=progress conv=fsync

echo "Kopiere Rootfs..."
dd if="$YOCTO_IMG_DIR/$ROOTFS_IMG" of="${LOOP_DEV}p2" bs=1M status=progress conv=fsync

echo "Synchronisiere und löse Loop-Device..."
sync
losetup -d "$LOOP_DEV"

echo "Image erfolgreich erstellt: $OUTPUT_IMG"
