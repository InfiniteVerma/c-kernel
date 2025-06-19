#include <kernel/allocator.h>
#include <kernel/gdt.h>
#include <kernel/interrupts.h>
#include <kernel/io/rtc.h>
#include <kernel/io/uart.h>
#include <kernel/multiboot.h>
#include <kernel/panic.h>
#include <kernel/tty.h>
#include <kernel/circular_buffer.h>
#include <utils.h>
#include "kernel/spinlock.h"

extern unsigned int get_esp();

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
    // unsigned int esp = get_esp();
    terminal_initialize();
    assert(init_serial() == 0, "Could not initialize serial port");
    // printf("Stack pointer: 0x%x\n", esp);
    LOG("Hello, kernel World, bootloader: %s", mbd->boot_loader_name);

#ifdef DEBUG
    parse_multiboot_info(mbd, magic);
#endif
    initialize_free_segments(mbd);
    configure_rtc();
    LOG("reading before lgdt done");
    read_gdt();
    init_gdt();
    LOG("After gdt!");
    LOG("reading after lgdt");
    read_gdt();
    init_idt();

    spinlock_t lock;
    lock.locked = 0;

    struct DateTime date_time = get_date_time();
    print_date_time(date_time);
    LOG("A%dA", 123);
    dump_buffer();

    // date_time.hours -= 1;
    // set_date_time(date_time);

    // LOG("After updating");
    // date_time = get_date_time();
    // print_date_time(date_time);
#ifdef TEST
    LOG("Starting tests");
    run_stdio_tests();
    run_allocator_tests();
    //run_gdt_tests(); TODO
    run_idt_tests();
    run_spinlock_tests();
    exit_(0);
#endif
}
