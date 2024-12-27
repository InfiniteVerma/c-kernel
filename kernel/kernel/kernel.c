#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/panic.h>
#include <kernel/allocator.h>

extern unsigned int get_esp();

#define DEBUG

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {

    unsigned int esp = get_esp();
	terminal_initialize();
    printf("Stack pointer: 0x%x\n", esp);
	printf("Hello, kernel World, bootloader: %s\n", mbd->boot_loader_name);
    
#ifdef DEBUG
    parse_multiboot_info(mbd, magic);
#endif
    initialize_free_segments(mbd);
    
    printf("DONE\n");
}
