#!/bin/sh
set -e
. ./iso.sh

echo $#

serial_flag="-serial stdio"
ram_flag="-m 512M"
exit_flag="-device isa-debug-exit,iobase=0xf4,iosize=0x04"

if [[ $# -eq 0 ]];
then
    echo "Starting without gdb"
    qemu-system-$(./target-triplet-to-arch.sh $HOST) $ram_flag $serial_flag $exit_flag -cdrom myos.iso -vnc :0 &
else
    echo "Starting with gdb"
    qemu-system-$(./target-triplet-to-arch.sh $HOST) -s -S -serial stdio -cdrom myos.iso -vnc :0 &
fi

sleep 1

vncviewer :0
