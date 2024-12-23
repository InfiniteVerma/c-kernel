#!/bin/sh
set -e
. ./iso.sh

#qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso
qemu-system-$(./target-triplet-to-arch.sh $HOST) -s -S -cdrom myos.iso -vnc :0 &

sleep 1

vncviewer :0
