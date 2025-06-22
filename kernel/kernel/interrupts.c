#include <kernel/interrupts.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/io/uart.h>
#include <utils.h>

struct interrupt_frame
{
    uint32_t instruction_pointer;
    uint32_t code_segment;
    uint32_t rflags;
    uint32_t register_stack_pointer;
    uint32_t stack_segment;
};

//void print_interrupt_frame(struct interrupt_frame* frame) {
//    LOG("instruction_pointer <0x%x>\n", (unsigned int)frame->instruction_pointer);
//    //LOG("code_segment %d", frame->code_segment);
//    //LOG("rflags %d", frame->rflags);
//    //LOG("register_stack_pointer %d", frame->register_stack_pointer);
//    //LOG("stack_segment %d", frame->stack_segment);
//}

typedef void (*InterruptFunc)(void);
typedef __attribute__((interrupt)) void(*InterruptHandlerFunc)(struct interrupt_frame*);

GateDescriptor interruptTable[256] = {0};
InterruptFunc interruptList[256] = {0};

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

//__attribute__((interrupt)) void interrupt_handler_55(struct interrupt_frame* frame)
//{
//    printf("interrupt_handler_55\n");
//}
//
#define DEFINE_INTERRUPT_HDLR_ERRNO(num) \
    __attribute__((interrupt)) void interrupt_handler_##num(struct interrupt_frame* frame, unsigned int){ \
        LOG("Inside interrupt handler " #num " - sip: 0x%x", frame->instruction_pointer); \
    }

#define DEFINE_INTERRUPT_HDLR(num) \
    __attribute__((interrupt)) void interrupt_handler_##num(struct interrupt_frame* frame) { \
        LOG("Inside interrupt handler " #num " - sip: 0x%x", frame->instruction_pointer); \
    }

DEFINE_INTERRUPT_HDLR(0)
DEFINE_INTERRUPT_HDLR(1)
DEFINE_INTERRUPT_HDLR(2)
DEFINE_INTERRUPT_HDLR(3)
DEFINE_INTERRUPT_HDLR(4)
DEFINE_INTERRUPT_HDLR(5)
DEFINE_INTERRUPT_HDLR(6)
DEFINE_INTERRUPT_HDLR(7)
DEFINE_INTERRUPT_HDLR(8)
DEFINE_INTERRUPT_HDLR(9)
DEFINE_INTERRUPT_HDLR(10)
DEFINE_INTERRUPT_HDLR(11)
DEFINE_INTERRUPT_HDLR(12)
DEFINE_INTERRUPT_HDLR_ERRNO(13)
DEFINE_INTERRUPT_HDLR(14)
DEFINE_INTERRUPT_HDLR(15)
DEFINE_INTERRUPT_HDLR(16)
DEFINE_INTERRUPT_HDLR(17)
DEFINE_INTERRUPT_HDLR(18)
DEFINE_INTERRUPT_HDLR(19)
DEFINE_INTERRUPT_HDLR(20)
DEFINE_INTERRUPT_HDLR(21)
DEFINE_INTERRUPT_HDLR(22)
DEFINE_INTERRUPT_HDLR(23)
DEFINE_INTERRUPT_HDLR(24)
DEFINE_INTERRUPT_HDLR(25)
DEFINE_INTERRUPT_HDLR(26)
DEFINE_INTERRUPT_HDLR(27)
DEFINE_INTERRUPT_HDLR(28)
DEFINE_INTERRUPT_HDLR(29)
DEFINE_INTERRUPT_HDLR(30)
DEFINE_INTERRUPT_HDLR(31)
DEFINE_INTERRUPT_HDLR(32)
DEFINE_INTERRUPT_HDLR(33)
DEFINE_INTERRUPT_HDLR(34)
DEFINE_INTERRUPT_HDLR(35)
DEFINE_INTERRUPT_HDLR(36)
DEFINE_INTERRUPT_HDLR(37)
DEFINE_INTERRUPT_HDLR(38)
DEFINE_INTERRUPT_HDLR(39)
DEFINE_INTERRUPT_HDLR(40)
DEFINE_INTERRUPT_HDLR(41)
DEFINE_INTERRUPT_HDLR(42)
DEFINE_INTERRUPT_HDLR(43)
DEFINE_INTERRUPT_HDLR(44)
DEFINE_INTERRUPT_HDLR(45)
DEFINE_INTERRUPT_HDLR(46)
DEFINE_INTERRUPT_HDLR(47)
DEFINE_INTERRUPT_HDLR(48)
DEFINE_INTERRUPT_HDLR(49)
DEFINE_INTERRUPT_HDLR(50)
DEFINE_INTERRUPT_HDLR(51)
DEFINE_INTERRUPT_HDLR(52)
DEFINE_INTERRUPT_HDLR(53)
DEFINE_INTERRUPT_HDLR(54)
DEFINE_INTERRUPT_HDLR(55)

void init_idt() {
    LOG("Initializing IDT");

    GateDescriptor gd;
    GateDescriptorNewArgs arg;

    InterruptHandlerFunc handlerFuncs[256] = {0};
    handlerFuncs[0] = interrupt_handler_0;
    handlerFuncs[1] = interrupt_handler_1;
    handlerFuncs[2] = interrupt_handler_2;
    handlerFuncs[3] = interrupt_handler_3;
    handlerFuncs[4] = interrupt_handler_4;
    handlerFuncs[5] = interrupt_handler_5;
    handlerFuncs[6] = interrupt_handler_6;
    handlerFuncs[7] = interrupt_handler_7;
    handlerFuncs[8] = interrupt_handler_8;
    handlerFuncs[9] = interrupt_handler_9;
    handlerFuncs[10] = interrupt_handler_10;
    handlerFuncs[11] = interrupt_handler_11;
    handlerFuncs[12] = interrupt_handler_12;
    //handlerFuncs[13] = interrupt_handler_13;
    handlerFuncs[14] = interrupt_handler_14;
    handlerFuncs[15] = interrupt_handler_15;
    handlerFuncs[16] = interrupt_handler_16;
    handlerFuncs[17] = interrupt_handler_17;
    handlerFuncs[18] = interrupt_handler_18;
    handlerFuncs[19] = interrupt_handler_19;
    handlerFuncs[20] = interrupt_handler_20;
    handlerFuncs[21] = interrupt_handler_21;
    handlerFuncs[22] = interrupt_handler_22;
    handlerFuncs[23] = interrupt_handler_23;
    handlerFuncs[24] = interrupt_handler_24;
    handlerFuncs[25] = interrupt_handler_25;
    handlerFuncs[26] = interrupt_handler_26;
    handlerFuncs[27] = interrupt_handler_27;
    handlerFuncs[28] = interrupt_handler_28;
    handlerFuncs[29] = interrupt_handler_29;
    handlerFuncs[30] = interrupt_handler_30;
    handlerFuncs[31] = interrupt_handler_31;
    handlerFuncs[32] = interrupt_handler_32;
    handlerFuncs[33] = interrupt_handler_33;
    handlerFuncs[34] = interrupt_handler_34;
    handlerFuncs[35] = interrupt_handler_35;
    handlerFuncs[36] = interrupt_handler_36;
    handlerFuncs[37] = interrupt_handler_37;
    handlerFuncs[38] = interrupt_handler_38;
    handlerFuncs[39] = interrupt_handler_39;
    handlerFuncs[40] = interrupt_handler_40;
    handlerFuncs[41] = interrupt_handler_41;
    handlerFuncs[42] = interrupt_handler_42;
    handlerFuncs[43] = interrupt_handler_43;
    handlerFuncs[44] = interrupt_handler_44;
    handlerFuncs[45] = interrupt_handler_45;
    handlerFuncs[46] = interrupt_handler_46;
    handlerFuncs[47] = interrupt_handler_47;
    handlerFuncs[48] = interrupt_handler_48;
    handlerFuncs[49] = interrupt_handler_49;
    handlerFuncs[50] = interrupt_handler_50;
    handlerFuncs[51] = interrupt_handler_51;
    handlerFuncs[52] = interrupt_handler_52;
    handlerFuncs[53] = interrupt_handler_53;
    handlerFuncs[54] = interrupt_handler_54;
    handlerFuncs[55] = interrupt_handler_55;

    outb(0x21, 0xff);
    outb(0xA1, 0xff);

    //for(int i=0;i<55;i++) {
    //    if(handlerFuncs[i] == NULL) {
    //        printf("i is NULL\n");
    //    }
    //    arg.offset = (uint32_t)(handlerFuncs[i]);
    //    arg.segment_selector = 0x8;
    //    arg.gate = 0b1111;
    //    arg.dpl = 0;
    //    arg.p = 1;

    //    gd = GateDescriptor_create(arg);

    //    interruptTable[i] = gd;
    //}


    {
    arg.offset = (uint32_t)(handlerFuncs[55]);
    arg.segment_selector = 0x8;
    arg.gate = 0b1111;
    arg.dpl = 0;
    arg.p = 1;

    gd = GateDescriptor_create(arg);

    interruptTable[55] = gd;
    }
//{
//    arg.offset = (uint32_t)(&interrupt_handler_13);
//    arg.segment_selector = 0x8;
//    arg.gate = 0b1111;
//    arg.dpl = 0;
//    arg.p = 1;
//
//    gd = GateDescriptor_create(arg);
//
//    interruptTable[13] = gd;
//    }

    Idt idt;
    idt.size = 256 * 8 - 1;
    idt.offset = (uint32_t)interruptTable;

    read_idt();
    LOG("Loading data: size: %d, offset: %d", idt.size, idt.offset);
    asm volatile(
            "lidt (%0)\n\t"
            "sti\n\t"
            "int $55\n\t"
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
