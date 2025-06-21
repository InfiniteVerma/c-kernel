#include <kernel/multiboot.h>
#include <utils.h>

extern unsigned long KERNEL_START;
extern unsigned long KERNEL_END;

void parse_multiboot_info(multiboot_info_t* mbd, unsigned int magic) {
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        LOG("invalid magic number");
        return;
    }

    if(!(mbd->flags >> 6 & 0x1)) {
        LOG("invalid memory map given by GRUB bootloader");
        return;
    }

    int i;
    long long size = 0;
    for(i = 0; i < mbd->mmap_length;
            i += sizeof(multiboot_memory_map_t))
    {
        multiboot_memory_map_t* mmmt = 
            (multiboot_memory_map_t*) (mbd->mmap_addr + i);

        LOG("Size: %u : Addr: 0x%x : Len %lluK : Type %u", mmmt->size, mmmt->addr, mmmt->len / 1024, mmmt->type);
        LOG("\n---\n");

        size += mmmt->len;
    }

    LOG("Total size: %lluM!\n", size / 1024 / 1024);

    LOG("Kernel start: 0x%x\n", &KERNEL_START);
    LOG("Kernel end: 0x%x\n", &KERNEL_END);
}
