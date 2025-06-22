#ifndef __INTERRUPTS__
#define __INTERRUPTS__

#include <stdbool.h>
#include <stdint.h>

#include "panic.h"

#define PIC_1_OFFSET 0x20
#define PIC_2_OFFSET 0x28

typedef void (*InterruptFunc)(void);

typedef struct {
    uint64_t descriptor;
} __attribute__((packed)) GateDescriptor;

typedef struct {
    uint32_t offset;
    bool p;
    uint8_t dpl;
    uint8_t gate;
    uint16_t segment_selector;
} GateDescriptorNewArgs;

typedef struct {
    uint16_t size;
    uint32_t offset;
} __attribute__((packed)) Idt;

GateDescriptor createNewGateDescriptor();

uint64_t generate_gd_entry(GateDescriptorNewArgs arg);

static inline GateDescriptor GateDescriptor_create(GateDescriptorNewArgs arg) {
    // then generate a uint64_t from it and assign it to gd
    GateDescriptor gd;
    gd.descriptor = generate_gd_entry(arg);
    return gd;
}

void init_idt();
void read_idt();
void register_interrupt(uint32_t interrupt_num, InterruptFunc interrupt_func);

#ifdef TEST
void run_idt_tests();
#endif

#endif
