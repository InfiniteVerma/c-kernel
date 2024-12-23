#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <string.h>

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
	printf("Hello, kernel World!\n");
    printf("Booting!\n");
    printf("Going to walk memories\n");

    long long size = 0;
    for(i = 0; i < mbd->mmap_length;
            i += sizeof(multiboot_memory_map_t))
    {
        multiboot_memory_map_t* mmmt = 
            (multiboot_memory_map_t*) (mbd->mmap_addr + i);

        printf("Start Addr: %d | Length: %d | Size: %d | Type: %d\n",
            mmmt->addr, mmmt->len, mmmt->size, mmmt->type);

        size += mmmt->size;
    }

    printf("Total size: %d!\n", size);
    const char* x = "ANANT";
    printf("%s\n", x);
    printf("%c\n", x[0]);
    //int written = printf("%d\n", 1234);
    //printf("%d\n", written);
    printf("%dABC\n", (int)strlen(x));
    //printf("\nDELIM\n");
    printf("%d\n", 42);
    //printf("world\n");
}
