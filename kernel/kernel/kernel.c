#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <string.h>

extern unsigned long KERNEL_START;
extern unsigned long KERNEL_END;

void panic(const char* msg) {
    printf("panicking");

    while (1) {
        asm volatile ("hlt");
    }
}

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {

	terminal_initialize();
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("invalid magic number\n");
        return;
    }

    if(!(mbd->flags >> 6 & 0x1)) {
        printf("invalid memory map given by GRUB bootloader");
        return;
    }

    int i;
	printf("Hello, kernel World, bootloader: %s\n", mbd->boot_loader_name);
    printf("First: %d, Second: %d, Third: %d, Fourth: %d\n", 123, 245, 456, 888);
    printf("First: %d, Second: %d, Third: %d, Fourth: %d\n", 123, 245, 456, 888);

    long long size = 0;
    for(i = 0; i < mbd->mmap_length;
            i += sizeof(multiboot_memory_map_t))
    {
        multiboot_memory_map_t* mmmt = 
            (multiboot_memory_map_t*) (mbd->mmap_addr + i);

        printf("Size: %u : Addr: %llu : Len %llu : Type %u", mmmt->size, mmmt->addr, mmmt->len, mmmt->type);
        printf("\n---\n");

        size += mmmt->len;
    }

    printf("Total size: <%d>M!\n", size / 1024 / 1024);
    printf("Kernel start: %d\n", &KERNEL_START);
    printf("Kernel end: %d\n", &KERNEL_END);
}
