#!/bin/sh
set -e
. ./iso.sh

echo $#

if [[ $# -eq 0 ]];
then
    echo "Starting without gdb"
    qemu-system-$(./target-triplet-to-arch.sh $HOST) -m 512M -cdrom myos.iso -vnc :0 &
else
    echo "Starting with gdb"
    qemu-system-$(./target-triplet-to-arch.sh $HOST) -s -S -cdrom myos.iso -vnc :0 &
fi

#qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso
#qemu-system-$(./target-triplet-to-arch.sh $HOST) -m 1G -s -S -cdrom myos.iso -vnc :0 &

sleep 1

vncviewer :0
