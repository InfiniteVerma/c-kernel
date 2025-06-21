#include <kernel/interrupts.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/io/uart.h>
#include <utils.h>

GateDescriptor interruptTable[256] = {0};

uint64_t generate_gd_entry(GateDescriptorNewArgs arg) {
    assert(arg.dpl <= (1 << 2), "arg.dpl should be <= 2 bits");
    assert(arg.gate <= (1 << 4), "arg.gate should be <= 4 bits");

    uint64_t ret = 0;

    ret |= arg.offset & 0xFFFF;
    
    ret |= (arg.segment_selector << 16);

    ret |= ((uint64_t)arg.gate << 40);

    ret |= ((uint64_t)arg.dpl << 45);

    ret |= ((uint64_t)arg.p << 47);

    ret |= ((((uint64_t)arg.offset >> 16) & 0xFFFF) << 48);

    return ret;
}

struct interrupt_frame;

__attribute__((interrupt)) void interrupt_handler(struct interrupt_frame* frame, unsigned long errorCode)
{
    LOG("INTERRUPT err: %d", errorCode);
}

void init_idt() {
    LOG("Initializing IDT");

    GateDescriptor gd;
    GateDescriptorNewArgs arg;

    // why needed?
    outb(0x21, 0xff);
    outb(0xA1, 0xff);

    arg.offset = (uint32_t)(&interrupt_handler);
    arg.segment_selector = 0x8;
    arg.gate = 0b1111;
    arg.dpl = 0;
    arg.p = 1;

    gd = GateDescriptor_create(arg);

    interruptTable[13] = gd;

    Idt idt;
    idt.size = 256 * 8 - 1;
    idt.offset = (uint32_t)interruptTable;

    read_idt();
    LOG("Loading data: size: %d, offset: %d", idt.size, idt.offset);
    asm volatile(
            "lidt (%0)\n\t"
            "sti\n\t"
            //"int $13\n\t"
            :
            : "r" (&idt)
            : "memory"
            );
    read_idt();
}

void read_idt() {
    LOG("read_idt BEGIN");
    Idt idt;
    idt.size = 1;
    asm volatile("sidt %0" : "=m"(idt));
    assert(idt.size != 1, "ERROR, sidt did not work properly");

    LOG("Size: %d - Offset: %d", idt.size, idt.offset);
}

#ifdef TEST
void test_basic() {
    GateDescriptorNewArgs args;
    args.offset = 0;
    args.p = 0;
    args.dpl = 0;
    args.gate = 0;
    args.segment_selector = 0;
    GateDescriptor descriptor = GateDescriptor_create(args);
    assert(descriptor.descriptor == 0, "test_basic of IDT failed");
}

void run_idt_tests() {
    test_basic();
    LOG_GREEN("Interrupt Descriptor Table: [OK]");
}
#endif
