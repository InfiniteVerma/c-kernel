#!/bin/sh
#set -e
#. ./iso.sh
. ./config.sh

#kill -9 $(pgrep qe)

serial_flag="-serial stdio"
ram_flag="-m 512M"
exit_flag="-device isa-debug-exit,iobase=0xf4,iosize=0x04"
pci_flag="-netdev user,id=n0 -device rtl8139,netdev=n0,bus=pci.0,addr=4,mac=12:34:56:78:9A:BC" # addr is in hex

if [[ $# -eq 0 ]];
then
    #echo "Starting without gdb"
    qemu-system-$(./target-triplet-to-arch.sh $HOST) $ram_flag $pci_flag $serial_flag $exit_flag -cdrom myos.iso -vnc :0 &
else
    #echo "Starting with gdb"
    qemu-system-$(./target-triplet-to-arch.sh $HOST) -s -S $pci_flag $serial_flag -cdrom myos.iso -vnc :0 &
fi

sleep 1

vncviewer :0
