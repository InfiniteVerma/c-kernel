#include <kernel/interrupts.h>
#include <kernel/io/rtc.h>
#include <kernel/io/uart.h>
#include <stdint.h>
#include <stdio.h>
#include <utils.h>

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

static void PIC_remap(int offset1, int offset2) {
    outb(PIC1_COMMAND,
         ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC1_DATA, offset1);  // ICW2: Master PIC vector offset
    outb(PIC2_DATA, offset2);  // ICW2: Slave PIC vector offset
    outb(PIC1_DATA, 4);  // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC2_DATA, 2);  // ICW3: tell Slave PIC its cascade identity (0000 0010)
    outb(PIC1_DATA, ICW4_8086);  // ICW4: have the PICs use 8086 mode (and not 8080 mode)
    outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

static void PIC_sendEOI(uint8_t irq) {
    const uint32_t PIC_EOI = 0x20;
    if (irq >= 8) outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}

static uint8_t PIC_remove_offset(uint32_t offset) {
    uint8_t irq = 0;
    if (offset >= PIC_2_OFFSET)
        irq = offset - PIC_2_OFFSET + 8;
    else if (offset >= PIC_1_OFFSET)
        irq = offset - PIC_1_OFFSET;
    else
        panic("Hardware interrupt, panic!");

    return irq;
}

struct interrupt_frame {
    uint32_t instruction_pointer;
    uint32_t code_segment;
    uint32_t rflags;
    uint32_t register_stack_pointer;
    uint32_t stack_segment;
};

typedef __attribute__((interrupt)) void (*InterruptHandlerFunc)(struct interrupt_frame*);
typedef __attribute__((interrupt)) void (*InterruptHandlerFuncErrNo)(struct interrupt_frame*,
                                                                     unsigned int);

GateDescriptor interruptTable[256] = {0};
InterruptFunc interruptList[256] = {0};  // TODO multithreaeded

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

// int type is important here because push instruction pushes 4 bytes so we cannot use uint8_t type
// here
void generic_interrupt_handler(int num) {
    if (interruptList[num] != NULL)
        interruptList[num]();
    else
        printf("unhandled Inside interrupt handler %d\n", num);
    PIC_sendEOI(PIC_remove_offset(num));
}

uint8_t INTERRUPT_MEM[256][21] = {0};

void generate_interrupt_stub(int num) {
    // 10220c:       55                      push   %ebp
    // 10220d:       89 e5                   mov    %esp,%ebp
    // 10220f:       60                      pusha
    // 102210:       6a 79                   push   $0x79
    // 102212:       e8 07 f4 ff ff          call   10161e <generic_interrupt_handler>
    // 102217:       83 c4 04                add    $0x4,%esp
    // 10221a:       61                      popa
    // 10221b:       89 ec                   mov    %ebp,%esp
    // 10221d:       5d                      pop    %ebp
    // 10221e:       cf                      iret
    // 10221f:       90                      nop

    const int NUM_IDX = 5;
    const int FUNCTION_ADDR_BASE = 7;

    uint8_t TEMPLATE[21] = {0x55, 0x89, 0xe5, 0x60, 0x6a, 0x7a, 0xb8, 0x1e, 0x16, 0x10, 0x00,
                            0xff, 0xd0, 0x83, 0xc4, 0x04, 0x61, 0x89, 0xec, 0x5d, 0xcf};
    TEMPLATE[NUM_IDX] = num;

    uint32_t func_addr = (uint32_t)generic_interrupt_handler;
    TEMPLATE[FUNCTION_ADDR_BASE] = func_addr & 0xFF;
    TEMPLATE[FUNCTION_ADDR_BASE + 1] = (func_addr >> 8) & 0xFF;
    TEMPLATE[FUNCTION_ADDR_BASE + 2] = (func_addr >> 16) & 0xFF;
    TEMPLATE[FUNCTION_ADDR_BASE + 3] = (func_addr >> 24) & 0xFF;

    for (int i = 0; i < 21; i++) {
        INTERRUPT_MEM[num][i] = TEMPLATE[i];
    }
}

// clang-format off
#define DEFINE_INTERRUPT_HDLR_V2(num)                                     \
    extern void interrupt_handler_##num();                                \
    __asm__(                                                              \
        ".align   4\n\t"                                                  \
        ".section .text\n\t"                                              \
        ".global interrupt_handler_" #num "\n\t"                          \
        ".type interrupt_handler_" #num ", @function\n\t"                 \
        "interrupt_handler_" #num ":\n\t"                                 \
        "   push %ebp\n\t"                                                \
        "   mov %esp, %ebp\n\t"                                           \
        "   pushal \n\t"                                                  \
        "   push $" #num "\n\t"                                           \
        "   mov $generic_interrupt_handler, %eax \n\t"                    \
        "   call %eax\n\t"                           \
        "   add $4, %esp\n\t"                                             \
        "   popal  \n\t"                                                  \
        "   mov %ebp, %esp\n\t"                                           \
        "   pop %ebp\n\t"                                                 \
        "   iret\n\t");
// clang-format on

void init_idt() {
    LOG("Initializing IDT");

    GateDescriptor gd;
    GateDescriptorNewArgs arg;

    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);

    PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);

    for (int i = 0; i < 256; i++) {
        generate_interrupt_stub(i);
        arg.offset = (uint32_t)INTERRUPT_MEM[i];
        arg.segment_selector = 0x8;
        arg.gate = 0b1111;
        arg.dpl = 0;
        arg.p = 1;

        gd = GateDescriptor_create(arg);

        interruptTable[i] = gd;
    }

    Idt idt;
    idt.size = 256 * 8 - 1;
    idt.offset = (uint32_t)interruptTable;

    read_idt();

    LOG("Loading data: size: %d, offset: %d", idt.size, idt.offset);
    asm volatile(
        "lidt (%0)\n\t"
        "sti\n\t"
        :
        : "r"(&idt)
        : "memory");

    read_idt();
}

// Expects the caller to disable interrupts first
void register_interrupt(uint32_t interrupt_num, InterruptFunc interrupt_func) {
    bool pic_2_needed = false;
    uint8_t pic_1_bit = 0, pic_2_bit = 0;

    if (interrupt_num >= PIC_2_OFFSET) {
        pic_2_needed = true;
        pic_2_bit = interrupt_num - PIC_2_OFFSET;
        pic_1_bit = 2;
    } else {
        pic_1_bit = interrupt_num - PIC_1_OFFSET;
    }

    uint8_t pic1_mask = inb(PIC1_DATA);
    outb(PIC1_DATA, pic1_mask & ~(1 << pic_1_bit));
    if (pic_2_needed) {
        uint8_t pic2_mask = inb(PIC2_DATA);
        outb(PIC2_DATA, pic2_mask & ~(1 << pic_2_bit));
    }

    interruptList[interrupt_num] = interrupt_func;
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
