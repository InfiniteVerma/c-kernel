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
        "   call generic_interrupt_handler\n\t"                           \
        "   add $4, %esp\n\t"                                             \
        "   popal  \n\t"                                                  \
        "   mov %ebp, %esp\n\t"                                           \
        "   pop %ebp\n\t"                                                 \
        "   iret\n\t");
// clang-format on

#define DEFINE_INTERRUPT_HDLR_ERRNO(num)                                                   \
    __attribute__((interrupt)) void interrupt_handler_##num(struct interrupt_frame* frame, \
                                                            unsigned int error_code) {     \
        printf("instruction ptr: %x\n", frame->instruction_pointer);                       \
        generic_interrupt_handler(num);                                                    \
    }

#define DEFINE_INTERRUPT_HDLR(num)                                                           \
    __attribute__((interrupt)) void interrupt_handler_##num(struct interrupt_frame* frame) { \
        printf("instruction ptr: %x\n", frame->instruction_pointer);                         \
        generic_interrupt_handler(num);                                                      \
    }

DEFINE_INTERRUPT_HDLR_V2(0)
DEFINE_INTERRUPT_HDLR_V2(1)
DEFINE_INTERRUPT_HDLR_V2(2)
DEFINE_INTERRUPT_HDLR_V2(3)
DEFINE_INTERRUPT_HDLR_V2(4)
DEFINE_INTERRUPT_HDLR_V2(5)
DEFINE_INTERRUPT_HDLR_V2(6)
DEFINE_INTERRUPT_HDLR_V2(7)
DEFINE_INTERRUPT_HDLR_ERRNO(8)
DEFINE_INTERRUPT_HDLR_V2(9)
DEFINE_INTERRUPT_HDLR_ERRNO(10)
DEFINE_INTERRUPT_HDLR_ERRNO(11)
DEFINE_INTERRUPT_HDLR_ERRNO(12)
DEFINE_INTERRUPT_HDLR_ERRNO(13)
DEFINE_INTERRUPT_HDLR_ERRNO(14)
DEFINE_INTERRUPT_HDLR_V2(15)
DEFINE_INTERRUPT_HDLR_V2(16)
DEFINE_INTERRUPT_HDLR_ERRNO(17)
DEFINE_INTERRUPT_HDLR_V2(18)
DEFINE_INTERRUPT_HDLR_V2(19)
DEFINE_INTERRUPT_HDLR_V2(20)
DEFINE_INTERRUPT_HDLR_ERRNO(21)
DEFINE_INTERRUPT_HDLR_V2(22)
DEFINE_INTERRUPT_HDLR_V2(23)
DEFINE_INTERRUPT_HDLR_V2(24)
DEFINE_INTERRUPT_HDLR_V2(25)
DEFINE_INTERRUPT_HDLR_V2(26)
DEFINE_INTERRUPT_HDLR_V2(27)
DEFINE_INTERRUPT_HDLR_V2(28)
DEFINE_INTERRUPT_HDLR_ERRNO(29)
DEFINE_INTERRUPT_HDLR_ERRNO(30)
DEFINE_INTERRUPT_HDLR_V2(31)
DEFINE_INTERRUPT_HDLR_V2(32)
DEFINE_INTERRUPT_HDLR_V2(33)
DEFINE_INTERRUPT_HDLR_V2(34)
DEFINE_INTERRUPT_HDLR_V2(35)
DEFINE_INTERRUPT_HDLR_V2(36)
DEFINE_INTERRUPT_HDLR_V2(37)
DEFINE_INTERRUPT_HDLR_V2(38)
DEFINE_INTERRUPT_HDLR_V2(39)
DEFINE_INTERRUPT_HDLR_V2(40)
DEFINE_INTERRUPT_HDLR_V2(41)
DEFINE_INTERRUPT_HDLR_V2(42)
DEFINE_INTERRUPT_HDLR_V2(43)
DEFINE_INTERRUPT_HDLR_V2(44)
DEFINE_INTERRUPT_HDLR_V2(45)
DEFINE_INTERRUPT_HDLR_V2(46)
DEFINE_INTERRUPT_HDLR_V2(47)
DEFINE_INTERRUPT_HDLR_V2(48)
DEFINE_INTERRUPT_HDLR_V2(49)
DEFINE_INTERRUPT_HDLR_V2(50)
DEFINE_INTERRUPT_HDLR_V2(51)
DEFINE_INTERRUPT_HDLR_V2(52)
DEFINE_INTERRUPT_HDLR_V2(53)
DEFINE_INTERRUPT_HDLR_V2(54)
DEFINE_INTERRUPT_HDLR_V2(55)
DEFINE_INTERRUPT_HDLR_V2(56)
DEFINE_INTERRUPT_HDLR_V2(57)
DEFINE_INTERRUPT_HDLR_V2(58)
DEFINE_INTERRUPT_HDLR_V2(59)
DEFINE_INTERRUPT_HDLR_V2(60)
DEFINE_INTERRUPT_HDLR_V2(61)
DEFINE_INTERRUPT_HDLR_V2(62)
DEFINE_INTERRUPT_HDLR_V2(63)
DEFINE_INTERRUPT_HDLR_V2(64)
DEFINE_INTERRUPT_HDLR_V2(65)
DEFINE_INTERRUPT_HDLR_V2(66)
DEFINE_INTERRUPT_HDLR_V2(67)
DEFINE_INTERRUPT_HDLR_V2(68)
DEFINE_INTERRUPT_HDLR_V2(69)
DEFINE_INTERRUPT_HDLR_V2(70)
DEFINE_INTERRUPT_HDLR_V2(71)
DEFINE_INTERRUPT_HDLR_V2(72)
DEFINE_INTERRUPT_HDLR_V2(73)
DEFINE_INTERRUPT_HDLR_V2(74)
DEFINE_INTERRUPT_HDLR_V2(75)
DEFINE_INTERRUPT_HDLR_V2(76)
DEFINE_INTERRUPT_HDLR_V2(77)
DEFINE_INTERRUPT_HDLR_V2(78)
DEFINE_INTERRUPT_HDLR_V2(79)
DEFINE_INTERRUPT_HDLR_V2(80)
DEFINE_INTERRUPT_HDLR_V2(81)
DEFINE_INTERRUPT_HDLR_V2(82)
DEFINE_INTERRUPT_HDLR_V2(83)
DEFINE_INTERRUPT_HDLR_V2(84)
DEFINE_INTERRUPT_HDLR_V2(85)
DEFINE_INTERRUPT_HDLR_V2(86)
DEFINE_INTERRUPT_HDLR_V2(87)
DEFINE_INTERRUPT_HDLR_V2(88)
DEFINE_INTERRUPT_HDLR_V2(89)
DEFINE_INTERRUPT_HDLR_V2(90)
DEFINE_INTERRUPT_HDLR_V2(91)
DEFINE_INTERRUPT_HDLR_V2(92)
DEFINE_INTERRUPT_HDLR_V2(93)
DEFINE_INTERRUPT_HDLR_V2(94)
DEFINE_INTERRUPT_HDLR_V2(95)
DEFINE_INTERRUPT_HDLR_V2(96)
DEFINE_INTERRUPT_HDLR_V2(97)
DEFINE_INTERRUPT_HDLR_V2(98)
DEFINE_INTERRUPT_HDLR_V2(99)
DEFINE_INTERRUPT_HDLR_V2(100)
DEFINE_INTERRUPT_HDLR_V2(101)
DEFINE_INTERRUPT_HDLR_V2(102)
DEFINE_INTERRUPT_HDLR_V2(103)
DEFINE_INTERRUPT_HDLR_V2(104)
DEFINE_INTERRUPT_HDLR_V2(105)
DEFINE_INTERRUPT_HDLR_V2(106)
DEFINE_INTERRUPT_HDLR_V2(107)
DEFINE_INTERRUPT_HDLR_V2(108)
DEFINE_INTERRUPT_HDLR_V2(109)
DEFINE_INTERRUPT_HDLR_V2(110)
DEFINE_INTERRUPT_HDLR_V2(111)
DEFINE_INTERRUPT_HDLR_V2(112)
DEFINE_INTERRUPT_HDLR_V2(113)
DEFINE_INTERRUPT_HDLR_V2(114)
DEFINE_INTERRUPT_HDLR_V2(115)
DEFINE_INTERRUPT_HDLR_V2(116)
DEFINE_INTERRUPT_HDLR_V2(117)
DEFINE_INTERRUPT_HDLR_V2(118)
DEFINE_INTERRUPT_HDLR_V2(119)
DEFINE_INTERRUPT_HDLR_V2(120)
DEFINE_INTERRUPT_HDLR_V2(121)
DEFINE_INTERRUPT_HDLR_V2(122)
DEFINE_INTERRUPT_HDLR_V2(123)
DEFINE_INTERRUPT_HDLR_V2(124)
DEFINE_INTERRUPT_HDLR_V2(125)
DEFINE_INTERRUPT_HDLR_V2(126)
DEFINE_INTERRUPT_HDLR_V2(127)
DEFINE_INTERRUPT_HDLR_V2(128)
DEFINE_INTERRUPT_HDLR_V2(129)
DEFINE_INTERRUPT_HDLR_V2(130)
DEFINE_INTERRUPT_HDLR_V2(131)
DEFINE_INTERRUPT_HDLR_V2(132)
DEFINE_INTERRUPT_HDLR_V2(133)
DEFINE_INTERRUPT_HDLR_V2(134)
DEFINE_INTERRUPT_HDLR_V2(135)
DEFINE_INTERRUPT_HDLR_V2(136)
DEFINE_INTERRUPT_HDLR_V2(137)
DEFINE_INTERRUPT_HDLR_V2(138)
DEFINE_INTERRUPT_HDLR_V2(139)
DEFINE_INTERRUPT_HDLR_V2(140)
DEFINE_INTERRUPT_HDLR_V2(141)
DEFINE_INTERRUPT_HDLR_V2(142)
DEFINE_INTERRUPT_HDLR_V2(143)
DEFINE_INTERRUPT_HDLR_V2(144)
DEFINE_INTERRUPT_HDLR_V2(145)
DEFINE_INTERRUPT_HDLR_V2(146)
DEFINE_INTERRUPT_HDLR_V2(147)
DEFINE_INTERRUPT_HDLR_V2(148)
DEFINE_INTERRUPT_HDLR_V2(149)
DEFINE_INTERRUPT_HDLR_V2(150)
DEFINE_INTERRUPT_HDLR_V2(151)
DEFINE_INTERRUPT_HDLR_V2(152)
DEFINE_INTERRUPT_HDLR_V2(153)
DEFINE_INTERRUPT_HDLR_V2(154)
DEFINE_INTERRUPT_HDLR_V2(155)
DEFINE_INTERRUPT_HDLR_V2(156)
DEFINE_INTERRUPT_HDLR_V2(157)
DEFINE_INTERRUPT_HDLR_V2(158)
DEFINE_INTERRUPT_HDLR_V2(159)
DEFINE_INTERRUPT_HDLR_V2(160)
DEFINE_INTERRUPT_HDLR_V2(161)
DEFINE_INTERRUPT_HDLR_V2(162)
DEFINE_INTERRUPT_HDLR_V2(163)
DEFINE_INTERRUPT_HDLR_V2(164)
DEFINE_INTERRUPT_HDLR_V2(165)
DEFINE_INTERRUPT_HDLR_V2(166)
DEFINE_INTERRUPT_HDLR_V2(167)
DEFINE_INTERRUPT_HDLR_V2(168)
DEFINE_INTERRUPT_HDLR_V2(169)
DEFINE_INTERRUPT_HDLR_V2(170)
DEFINE_INTERRUPT_HDLR_V2(171)
DEFINE_INTERRUPT_HDLR_V2(172)
DEFINE_INTERRUPT_HDLR_V2(173)
DEFINE_INTERRUPT_HDLR_V2(174)
DEFINE_INTERRUPT_HDLR_V2(175)
DEFINE_INTERRUPT_HDLR_V2(176)
DEFINE_INTERRUPT_HDLR_V2(177)
DEFINE_INTERRUPT_HDLR_V2(178)
DEFINE_INTERRUPT_HDLR_V2(179)
DEFINE_INTERRUPT_HDLR_V2(180)
DEFINE_INTERRUPT_HDLR_V2(181)
DEFINE_INTERRUPT_HDLR_V2(182)
DEFINE_INTERRUPT_HDLR_V2(183)
DEFINE_INTERRUPT_HDLR_V2(184)
DEFINE_INTERRUPT_HDLR_V2(185)
DEFINE_INTERRUPT_HDLR_V2(186)
DEFINE_INTERRUPT_HDLR_V2(187)
DEFINE_INTERRUPT_HDLR_V2(188)
DEFINE_INTERRUPT_HDLR_V2(189)
DEFINE_INTERRUPT_HDLR_V2(190)
DEFINE_INTERRUPT_HDLR_V2(191)
DEFINE_INTERRUPT_HDLR_V2(192)
DEFINE_INTERRUPT_HDLR_V2(193)
DEFINE_INTERRUPT_HDLR_V2(194)
DEFINE_INTERRUPT_HDLR_V2(195)
DEFINE_INTERRUPT_HDLR_V2(196)
DEFINE_INTERRUPT_HDLR_V2(197)
DEFINE_INTERRUPT_HDLR_V2(198)
DEFINE_INTERRUPT_HDLR_V2(199)
DEFINE_INTERRUPT_HDLR_V2(200)
DEFINE_INTERRUPT_HDLR_V2(201)
DEFINE_INTERRUPT_HDLR_V2(202)
DEFINE_INTERRUPT_HDLR_V2(203)
DEFINE_INTERRUPT_HDLR_V2(204)
DEFINE_INTERRUPT_HDLR_V2(205)
DEFINE_INTERRUPT_HDLR_V2(206)
DEFINE_INTERRUPT_HDLR_V2(207)
DEFINE_INTERRUPT_HDLR_V2(208)
DEFINE_INTERRUPT_HDLR_V2(209)
DEFINE_INTERRUPT_HDLR_V2(210)
DEFINE_INTERRUPT_HDLR_V2(211)
DEFINE_INTERRUPT_HDLR_V2(212)
DEFINE_INTERRUPT_HDLR_V2(213)
DEFINE_INTERRUPT_HDLR_V2(214)
DEFINE_INTERRUPT_HDLR_V2(215)
DEFINE_INTERRUPT_HDLR_V2(216)
DEFINE_INTERRUPT_HDLR_V2(217)
DEFINE_INTERRUPT_HDLR_V2(218)
DEFINE_INTERRUPT_HDLR_V2(219)
DEFINE_INTERRUPT_HDLR_V2(220)
DEFINE_INTERRUPT_HDLR_V2(221)
DEFINE_INTERRUPT_HDLR_V2(222)
DEFINE_INTERRUPT_HDLR_V2(223)
DEFINE_INTERRUPT_HDLR_V2(224)
DEFINE_INTERRUPT_HDLR_V2(225)
DEFINE_INTERRUPT_HDLR_V2(226)
DEFINE_INTERRUPT_HDLR_V2(227)
DEFINE_INTERRUPT_HDLR_V2(228)
DEFINE_INTERRUPT_HDLR_V2(229)
DEFINE_INTERRUPT_HDLR_V2(230)
DEFINE_INTERRUPT_HDLR_V2(231)
DEFINE_INTERRUPT_HDLR_V2(232)
DEFINE_INTERRUPT_HDLR_V2(233)
DEFINE_INTERRUPT_HDLR_V2(234)
DEFINE_INTERRUPT_HDLR_V2(235)
DEFINE_INTERRUPT_HDLR_V2(236)
DEFINE_INTERRUPT_HDLR_V2(237)
DEFINE_INTERRUPT_HDLR_V2(238)
DEFINE_INTERRUPT_HDLR_V2(239)
DEFINE_INTERRUPT_HDLR_V2(240)
DEFINE_INTERRUPT_HDLR_V2(241)
DEFINE_INTERRUPT_HDLR_V2(242)
DEFINE_INTERRUPT_HDLR_V2(243)
DEFINE_INTERRUPT_HDLR_V2(244)
DEFINE_INTERRUPT_HDLR_V2(245)
DEFINE_INTERRUPT_HDLR_V2(246)
DEFINE_INTERRUPT_HDLR_V2(247)
DEFINE_INTERRUPT_HDLR_V2(248)
DEFINE_INTERRUPT_HDLR_V2(249)
DEFINE_INTERRUPT_HDLR_V2(250)
DEFINE_INTERRUPT_HDLR_V2(251)
DEFINE_INTERRUPT_HDLR_V2(252)
DEFINE_INTERRUPT_HDLR_V2(253)
DEFINE_INTERRUPT_HDLR_V2(254)
DEFINE_INTERRUPT_HDLR_V2(255)

void init_idt() {
    LOG("Initializing IDT");

    GateDescriptor gd;
    GateDescriptorNewArgs arg;

    // TODO fix sizes. 256 array size is not needed
    InterruptHandlerFunc handlerFuncs[256] = {0};
    InterruptHandlerFuncErrNo handlerFuncsErrNo[256] = {0};

    handlerFuncs[0] = interrupt_handler_0;
    handlerFuncs[1] = interrupt_handler_1;
    handlerFuncs[2] = interrupt_handler_2;
    handlerFuncs[3] = interrupt_handler_3;
    handlerFuncs[4] = interrupt_handler_4;
    handlerFuncs[5] = interrupt_handler_5;
    handlerFuncs[6] = interrupt_handler_6;
    handlerFuncs[7] = interrupt_handler_7;
    handlerFuncsErrNo[8] = interrupt_handler_8;
    handlerFuncs[9] = interrupt_handler_9;
    handlerFuncsErrNo[10] = interrupt_handler_10;
    handlerFuncsErrNo[11] = interrupt_handler_11;
    handlerFuncsErrNo[12] = interrupt_handler_12;
    handlerFuncsErrNo[13] = interrupt_handler_13;
    handlerFuncsErrNo[14] = interrupt_handler_14;
    handlerFuncs[15] = interrupt_handler_15;
    handlerFuncs[16] = interrupt_handler_16;
    handlerFuncsErrNo[17] = interrupt_handler_17;
    handlerFuncs[18] = interrupt_handler_18;
    handlerFuncs[19] = interrupt_handler_19;
    handlerFuncs[20] = interrupt_handler_20;
    handlerFuncsErrNo[21] = interrupt_handler_21;
    handlerFuncs[22] = interrupt_handler_22;
    handlerFuncs[23] = interrupt_handler_23;
    handlerFuncs[24] = interrupt_handler_24;
    handlerFuncs[25] = interrupt_handler_25;
    handlerFuncs[26] = interrupt_handler_26;
    handlerFuncs[27] = interrupt_handler_27;
    handlerFuncs[28] = interrupt_handler_28;
    handlerFuncsErrNo[29] = interrupt_handler_29;
    handlerFuncsErrNo[30] = interrupt_handler_30;
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
    handlerFuncs[56] = interrupt_handler_56;
    handlerFuncs[57] = interrupt_handler_57;
    handlerFuncs[58] = interrupt_handler_58;
    handlerFuncs[59] = interrupt_handler_59;
    handlerFuncs[60] = interrupt_handler_60;
    handlerFuncs[61] = interrupt_handler_61;
    handlerFuncs[62] = interrupt_handler_62;
    handlerFuncs[63] = interrupt_handler_63;
    handlerFuncs[64] = interrupt_handler_64;
    handlerFuncs[65] = interrupt_handler_65;
    handlerFuncs[66] = interrupt_handler_66;
    handlerFuncs[67] = interrupt_handler_67;
    handlerFuncs[68] = interrupt_handler_68;
    handlerFuncs[69] = interrupt_handler_69;
    handlerFuncs[70] = interrupt_handler_70;
    handlerFuncs[71] = interrupt_handler_71;
    handlerFuncs[72] = interrupt_handler_72;
    handlerFuncs[73] = interrupt_handler_73;
    handlerFuncs[74] = interrupt_handler_74;
    handlerFuncs[75] = interrupt_handler_75;
    handlerFuncs[76] = interrupt_handler_76;
    handlerFuncs[77] = interrupt_handler_77;
    handlerFuncs[78] = interrupt_handler_78;
    handlerFuncs[79] = interrupt_handler_79;
    handlerFuncs[80] = interrupt_handler_80;
    handlerFuncs[81] = interrupt_handler_81;
    handlerFuncs[82] = interrupt_handler_82;
    handlerFuncs[83] = interrupt_handler_83;
    handlerFuncs[84] = interrupt_handler_84;
    handlerFuncs[85] = interrupt_handler_85;
    handlerFuncs[86] = interrupt_handler_86;
    handlerFuncs[87] = interrupt_handler_87;
    handlerFuncs[88] = interrupt_handler_88;
    handlerFuncs[89] = interrupt_handler_89;
    handlerFuncs[90] = interrupt_handler_90;
    handlerFuncs[91] = interrupt_handler_91;
    handlerFuncs[92] = interrupt_handler_92;
    handlerFuncs[93] = interrupt_handler_93;
    handlerFuncs[94] = interrupt_handler_94;
    handlerFuncs[95] = interrupt_handler_95;
    handlerFuncs[96] = interrupt_handler_96;
    handlerFuncs[97] = interrupt_handler_97;
    handlerFuncs[98] = interrupt_handler_98;
    handlerFuncs[99] = interrupt_handler_99;
    handlerFuncs[100] = interrupt_handler_100;
    handlerFuncs[101] = interrupt_handler_101;
    handlerFuncs[102] = interrupt_handler_102;
    handlerFuncs[103] = interrupt_handler_103;
    handlerFuncs[104] = interrupt_handler_104;
    handlerFuncs[105] = interrupt_handler_105;
    handlerFuncs[106] = interrupt_handler_106;
    handlerFuncs[107] = interrupt_handler_107;
    handlerFuncs[108] = interrupt_handler_108;
    handlerFuncs[109] = interrupt_handler_109;
    handlerFuncs[110] = interrupt_handler_110;
    handlerFuncs[111] = interrupt_handler_111;
    handlerFuncs[112] = interrupt_handler_112;
    handlerFuncs[113] = interrupt_handler_113;
    handlerFuncs[114] = interrupt_handler_114;
    handlerFuncs[115] = interrupt_handler_115;
    handlerFuncs[116] = interrupt_handler_116;
    handlerFuncs[117] = interrupt_handler_117;
    handlerFuncs[118] = interrupt_handler_118;
    handlerFuncs[119] = interrupt_handler_119;
    handlerFuncs[120] = interrupt_handler_120;
    handlerFuncs[121] = interrupt_handler_121;
    handlerFuncs[122] = interrupt_handler_122;
    handlerFuncs[123] = interrupt_handler_123;
    handlerFuncs[124] = interrupt_handler_124;
    handlerFuncs[125] = interrupt_handler_125;
    handlerFuncs[126] = interrupt_handler_126;
    handlerFuncs[127] = interrupt_handler_127;
    handlerFuncs[128] = interrupt_handler_128;
    handlerFuncs[129] = interrupt_handler_129;
    handlerFuncs[130] = interrupt_handler_130;
    handlerFuncs[131] = interrupt_handler_131;
    handlerFuncs[132] = interrupt_handler_132;
    handlerFuncs[133] = interrupt_handler_133;
    handlerFuncs[134] = interrupt_handler_134;
    handlerFuncs[135] = interrupt_handler_135;
    handlerFuncs[136] = interrupt_handler_136;
    handlerFuncs[137] = interrupt_handler_137;
    handlerFuncs[138] = interrupt_handler_138;
    handlerFuncs[139] = interrupt_handler_139;
    handlerFuncs[140] = interrupt_handler_140;
    handlerFuncs[141] = interrupt_handler_141;
    handlerFuncs[142] = interrupt_handler_142;
    handlerFuncs[143] = interrupt_handler_143;
    handlerFuncs[144] = interrupt_handler_144;
    handlerFuncs[145] = interrupt_handler_145;
    handlerFuncs[146] = interrupt_handler_146;
    handlerFuncs[147] = interrupt_handler_147;
    handlerFuncs[148] = interrupt_handler_148;
    handlerFuncs[149] = interrupt_handler_149;
    handlerFuncs[150] = interrupt_handler_150;
    handlerFuncs[151] = interrupt_handler_151;
    handlerFuncs[152] = interrupt_handler_152;
    handlerFuncs[153] = interrupt_handler_153;
    handlerFuncs[154] = interrupt_handler_154;
    handlerFuncs[155] = interrupt_handler_155;
    handlerFuncs[156] = interrupt_handler_156;
    handlerFuncs[157] = interrupt_handler_157;
    handlerFuncs[158] = interrupt_handler_158;
    handlerFuncs[159] = interrupt_handler_159;
    handlerFuncs[160] = interrupt_handler_160;
    handlerFuncs[161] = interrupt_handler_161;
    handlerFuncs[162] = interrupt_handler_162;
    handlerFuncs[163] = interrupt_handler_163;
    handlerFuncs[164] = interrupt_handler_164;
    handlerFuncs[165] = interrupt_handler_165;
    handlerFuncs[166] = interrupt_handler_166;
    handlerFuncs[167] = interrupt_handler_167;
    handlerFuncs[168] = interrupt_handler_168;
    handlerFuncs[169] = interrupt_handler_169;
    handlerFuncs[170] = interrupt_handler_170;
    handlerFuncs[171] = interrupt_handler_171;
    handlerFuncs[172] = interrupt_handler_172;
    handlerFuncs[173] = interrupt_handler_173;
    handlerFuncs[174] = interrupt_handler_174;
    handlerFuncs[175] = interrupt_handler_175;
    handlerFuncs[176] = interrupt_handler_176;
    handlerFuncs[177] = interrupt_handler_177;
    handlerFuncs[178] = interrupt_handler_178;
    handlerFuncs[179] = interrupt_handler_179;
    handlerFuncs[180] = interrupt_handler_180;
    handlerFuncs[181] = interrupt_handler_181;
    handlerFuncs[182] = interrupt_handler_182;
    handlerFuncs[183] = interrupt_handler_183;
    handlerFuncs[184] = interrupt_handler_184;
    handlerFuncs[185] = interrupt_handler_185;
    handlerFuncs[186] = interrupt_handler_186;
    handlerFuncs[187] = interrupt_handler_187;
    handlerFuncs[188] = interrupt_handler_188;
    handlerFuncs[189] = interrupt_handler_189;
    handlerFuncs[190] = interrupt_handler_190;
    handlerFuncs[191] = interrupt_handler_191;
    handlerFuncs[192] = interrupt_handler_192;
    handlerFuncs[193] = interrupt_handler_193;
    handlerFuncs[194] = interrupt_handler_194;
    handlerFuncs[195] = interrupt_handler_195;
    handlerFuncs[196] = interrupt_handler_196;
    handlerFuncs[197] = interrupt_handler_197;
    handlerFuncs[198] = interrupt_handler_198;
    handlerFuncs[199] = interrupt_handler_199;
    handlerFuncs[200] = interrupt_handler_200;
    handlerFuncs[201] = interrupt_handler_201;
    handlerFuncs[202] = interrupt_handler_202;
    handlerFuncs[203] = interrupt_handler_203;
    handlerFuncs[204] = interrupt_handler_204;
    handlerFuncs[205] = interrupt_handler_205;
    handlerFuncs[206] = interrupt_handler_206;
    handlerFuncs[207] = interrupt_handler_207;
    handlerFuncs[208] = interrupt_handler_208;
    handlerFuncs[209] = interrupt_handler_209;
    handlerFuncs[210] = interrupt_handler_210;
    handlerFuncs[211] = interrupt_handler_211;
    handlerFuncs[212] = interrupt_handler_212;
    handlerFuncs[213] = interrupt_handler_213;
    handlerFuncs[214] = interrupt_handler_214;
    handlerFuncs[215] = interrupt_handler_215;
    handlerFuncs[216] = interrupt_handler_216;
    handlerFuncs[217] = interrupt_handler_217;
    handlerFuncs[218] = interrupt_handler_218;
    handlerFuncs[219] = interrupt_handler_219;
    handlerFuncs[220] = interrupt_handler_220;
    handlerFuncs[221] = interrupt_handler_221;
    handlerFuncs[222] = interrupt_handler_222;
    handlerFuncs[223] = interrupt_handler_223;
    handlerFuncs[224] = interrupt_handler_224;
    handlerFuncs[225] = interrupt_handler_225;
    handlerFuncs[226] = interrupt_handler_226;
    handlerFuncs[227] = interrupt_handler_227;
    handlerFuncs[228] = interrupt_handler_228;
    handlerFuncs[229] = interrupt_handler_229;
    handlerFuncs[230] = interrupt_handler_230;
    handlerFuncs[231] = interrupt_handler_231;
    handlerFuncs[232] = interrupt_handler_232;
    handlerFuncs[233] = interrupt_handler_233;
    handlerFuncs[234] = interrupt_handler_234;
    handlerFuncs[235] = interrupt_handler_235;
    handlerFuncs[236] = interrupt_handler_236;
    handlerFuncs[237] = interrupt_handler_237;
    handlerFuncs[238] = interrupt_handler_238;
    handlerFuncs[239] = interrupt_handler_239;
    handlerFuncs[240] = interrupt_handler_240;
    handlerFuncs[241] = interrupt_handler_241;
    handlerFuncs[242] = interrupt_handler_242;
    handlerFuncs[243] = interrupt_handler_243;
    handlerFuncs[244] = interrupt_handler_244;
    handlerFuncs[245] = interrupt_handler_245;
    handlerFuncs[246] = interrupt_handler_246;
    handlerFuncs[247] = interrupt_handler_247;
    handlerFuncs[248] = interrupt_handler_248;
    handlerFuncs[249] = interrupt_handler_249;
    handlerFuncs[250] = interrupt_handler_250;
    handlerFuncs[251] = interrupt_handler_251;
    handlerFuncs[252] = interrupt_handler_252;
    handlerFuncs[253] = interrupt_handler_253;
    handlerFuncs[254] = interrupt_handler_254;
    handlerFuncs[255] = interrupt_handler_255;

    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);

    PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);

    for (int i = 0; i < 256; i++) {
        if (handlerFuncs[i] == NULL && handlerFuncsErrNo[i] == NULL) {
            LOG("SKIPPING: %d", i);
            continue;
        }
        if (handlerFuncs[i] != NULL) {
            arg.offset = (uint32_t)(handlerFuncs[i]);
        } else {
            arg.offset = (uint32_t)(handlerFuncsErrNo[i]);
        }
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
