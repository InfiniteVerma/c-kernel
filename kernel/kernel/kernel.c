#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/panic.h>
#include <kernel/allocator.h>
#include <kernel/io/uart.h>

extern unsigned int get_esp();

#ifdef TEST
#endif

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {

    //unsigned int esp = get_esp();
	terminal_initialize();
    assert(init_serial() == 0, "Could not initialize serial port");
    //printf("Stack pointer: 0x%x\n", esp);
	printf("Hello, kernel World, bootloader: %s\n", mbd->boot_loader_name);
    
#ifdef DEBUG
    parse_multiboot_info(mbd, magic);
#endif
    initialize_free_segments(mbd);

#ifdef TEST
    printf("Starting tests\n");
    run_allocator_tests();
#endif
    
    printf("DONE\n");
    //exit_(1);
}
