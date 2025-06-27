#include <kernel/allocator.h>
#include <kernel/circular_buffer.h>
#include <kernel/future.h>
#include <kernel/gdt.h>
#include <kernel/interrupts.h>
#include <kernel/io/rtc.h>
#include <kernel/io/uart.h>
#include <kernel/monotonic_tick.h>
#include <kernel/multiboot.h>
#include <kernel/panic.h>
#include <kernel/tty.h>
#include <stdio.h>
#include <unistd.h>
#include <utils.h>
#ifdef TEST
#include <kernel/spinlock.h>
#include <stdio.h>
#include <utils.h>
#endif

bool fut_is_ready(void* ctx) {
    uint32_t* needed_tick = (uint32_t*)ctx;

    if (get_tick() < *needed_tick) return false;
    return true;
}

void fut_resume_func(void* ctx) {
    printf("resume func called\n");
}

extern unsigned int get_esp();

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
    // unsigned int esp = get_esp();
    terminal_initialize();

    init_gdt();
    read_gdt();
    init_idt();

    assert(init_serial() == 0, "Could not initialize serial port");
    // printf("Stack pointer: 0x%x\n", esp);
    LOG("Hello, kernel World, bootloader: %s", mbd->boot_loader_name);

#ifdef DEBUG
    parse_multiboot_info(mbd, magic);
#endif
    initialize_free_segments(mbd);
    configure_rtc();

#ifdef DEBUG
    LOG("reading before lgdt done");
    read_gdt();
#endif
    struct DateTime date_time = get_date_time();
    print_date_time(date_time);

    // #ifndef TEST
    //     LOG("Sleeping for 4 seconds");
    //     dump_buffer();
    //     sleep(4);
    //     LOG("Waking up");
    // #endif
    init_futures();

    // date_time.hours -= 1;
    // set_date_time(date_time);

    // LOG("After updating");
    // date_time = get_date_time();
    // print_date_time(date_time);
#ifdef TEST
    LOG_GREEN("Starting tests");
    run_utils_tests();
    run_stdio_tests();
    run_allocator_tests();
    // run_gdt_tests(); TODO
    run_idt_tests();
    run_spinlock_tests();
    run_rtc_tests();
#endif

    serial_write("anant", 5);

    LOG("Sleeping for 4 seconds");
    dump_buffer();
    Future fut = create_future(4, fut_is_ready, fut_resume_func);
    await(fut);
    LOG("Waking up");
    dump_buffer();

#ifdef TEST
    exit_(0);
#endif
    while (1) {
    };
}
