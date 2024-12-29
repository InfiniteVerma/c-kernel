#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/panic.h>
#include <kernel/allocator.h>
#include <kernel/gdt.h>
#include <kernel/io/uart.h>
#include <kernel/io/rtc.h>

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

    configure_rtc();

#ifdef TEST
    printf("Starting tests\n");
    run_allocator_tests();
    run_gdt_tests();
#endif
    
    //struct DateTime date_time = get_date_time();
    //print_date_time(date_time);

    //date_time.hours -= 1;
    //set_date_time(date_time);

    //printf("After updating\n");
    //date_time = get_date_time();
    //print_date_time(date_time);
#ifdef TEST
    exit_(0);
#endif
}
