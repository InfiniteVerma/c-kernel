#include <kernel/gdt.h>
#include <kernel/panic.h>
#include <stdio.h>

#define FLAGS 1100

uint64_t gdt[3] = {0}; // has to be global memory. This needs to stay alive.

static const uint64_t gdt_parse_base(uint64_t segment) {
    uint64_t ret = 0;

    const uint64_t BASE_LOWER_MASK = (uint64_t)0xFFFFFF << 16;
    const uint64_t BASE_UPPER_MASK = (uint64_t)0xFF << 56;

    ret |= (segment & BASE_LOWER_MASK) >> 16;
    ret |= (segment & BASE_UPPER_MASK) >> (56 - (16 + 8));

    return ret;
}

static const uint64_t gdt_parse_limit(uint64_t segment) {
    uint64_t ret = 0;

    const uint64_t LIMIT_LOWER_MASK = 0xFFFF;
    const uint64_t LIMIT_UPPER_MASK = ((uint64_t)0xF << 48);

    assert(LIMIT_UPPER_MASK == 0xF000000000000, "Doesn't match");

    ret |= (segment & LIMIT_LOWER_MASK);
    ret |= (segment & LIMIT_UPPER_MASK) >> 32;

    return ret;
}

static void set_p(struct AccessByte* access_byte, const uint8_t val) {
    assert((val >> 1) == 0, "Not one bit");
    access_byte->p = val;
}

static void set_dpl(struct AccessByte* access_byte, const uint8_t val) {
    assert((val >> 2) == 0, "Not two bits");
    access_byte->dpl = val;
}

static void set_s(struct AccessByte* access_byte, const uint8_t val) {
    assert((val >> 1) == 0, "Not one bit");
    access_byte->s = val;
}

static void set_e(struct AccessByte* access_byte, const uint8_t val) {
    assert((val >> 1) == 0, "Not one bit");
    access_byte->e = val;
}

static void set_dc(struct AccessByte* access_byte, const uint8_t val) {
    assert((val >> 1) == 0, "Not one bit");
    access_byte->dc = val;
}

static void set_rw(struct AccessByte* access_byte, const uint8_t val) {
    assert((val >> 1) == 0, "Not one bit");
    access_byte->rw = val;
}

static void set_a(struct AccessByte* access_byte, const uint8_t val) {
    assert((val >> 1) == 0, "Not one bit");
    access_byte->a = val;
}

static const uint8_t get_binary_from_access_byte(const struct AccessByte access_byte) {
    uint8_t ret = 0;

    ret |= (access_byte.a << 0);            // Accessed
    ret |= (access_byte.rw << 1);           // Readable/Writable
    ret |= (access_byte.dc << 2);           // Direction/Conforming
    ret |= (access_byte.e << 3);            // Executable
    ret |= (access_byte.s << 4);            // Descriptor type
    ret |= ((access_byte.dpl & 0x3) << 5);  // DPL (2 bits)
    ret |= (access_byte.p << 7);            // Present

    return ret;
}

static uint64_t insert_base(uint32_t base) {
    uint64_t ret = 0;

    ret |= (base & 0xFFFFFF) << 16;
    ret |= ((uint64_t)base >> 24) << 56;

    return ret;
}

static uint64_t insert_limit(uint32_t limit) {
    uint64_t ret = 0;

    assert(limit >> 20 == 0, "Limit is > 20 bit, not allowed");

    ret |= (limit & 0xFFFF);
    ret |= (((uint64_t)limit & 0xF0000) >> 16) << 48;

    return ret;
}

static uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flags) {
    uint64_t descriptor = 0;

    // Low 16 bits of limit
    descriptor |= (limit & 0xFFFFULL);

    // Low 16 bits of base
    descriptor |= ((base & 0xFFFFULL) << 16);

    // Next 8 bits of base (bits 23:16)
    descriptor |= ((base >> 16) & 0xFFULL) << 32;

    // Access byte (low 8 bits of flags)
    descriptor |= ((flags & 0xFFULL) << 40);

    // High 4 bits of limit (bits 19:16)
    descriptor |= ((limit >> 16) & 0x0FULL) << 48;

    // Flags (high 4 bits of flags)
    descriptor |= ((flags >> 8) & 0x0FULL) << 52;

    // Highest 8 bits of base (bits 31:24)
    descriptor |= ((base >> 24) & 0xFFULL) << 56;

    return descriptor;
}

static void fill_gdt_vals() {
    struct AccessByte code_access_byte;
    set_p(&code_access_byte, 1);
    set_dpl(&code_access_byte, 0);
    set_s(&code_access_byte, 1);
    set_e(&code_access_byte, 1);
    set_dc(&code_access_byte, 0);
    set_rw(&code_access_byte, 0);
    set_a(&code_access_byte, 1);

    uint16_t flags = (0x1101 << 8) | get_binary_from_access_byte(code_access_byte);

    const uint64_t code_segment = create_descriptor(0, 0xffffffff, flags);

    struct AccessByte data_access_byte;
    set_p(&data_access_byte, 1);
    set_dpl(&data_access_byte, 0);
    set_s(&data_access_byte, 1);
    set_e(&data_access_byte, 0);
    set_dc(&data_access_byte, 0);
    set_rw(&data_access_byte, 1);
    set_a(&data_access_byte, 1);

    // printf("data_access_byte: %d\n", get_binary_from_access_byte(data_access_byte));

    flags = (0x1101 << 8) | get_binary_from_access_byte(data_access_byte);
    const uint64_t data_segment = create_descriptor(0, 0xffffffff, flags);

    const uint64_t null_segment = 0;

    // TODO buggy
    //gdt[0] = null_segment;
    //gdt[1] = code_segment;
    //gdt[2] = data_segment;
    gdt[0] = 0x0000000000000000;
    gdt[1] = 0x00CF9A000000FFFF;
    gdt[2] = 0x00CF92000000FFFF;
    assert(code_segment != 0, "code_Segment is zero!");
    printf("Loading null: %x\ncode: %x\ndata: %x\n", null_segment, code_segment, data_segment);
}

void init_gdt() {
    fill_gdt_vals();

    GDTDescriptor gdtr;
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)gdt;

    asm volatile(
            "cli\n\t"
            "lgdt (%0)\n\t"
            "ljmp $0x08, $1f\n\t" // using inline 1j preserves stack frame. Using $reload_CS like in tutorial does not
            "1:\n\t"
            "mov $0x10, %%eax\n\t"
            "mov %%eax, %%ds\n\t"
            "mov %%eax, %%es\n\t"
            "mov %%eax, %%fs\n\t"
            "mov %%eax, %%gs\n\t"
            "mov %%eax, %%ss\n\t"
            :  // No output operands
            : "r"(&gdtr)
            : "memory");
}

void read_gdt() {
    printf("read_gdt BEGIN\n");
    GDTDescriptor gdtr;
    asm volatile("sgdt %0" : "=m"(gdtr));

    uint64_t* gdt_tmp = (uint64_t*)(gdtr.base);
    printf("GDT Limit: 0x%x\n", gdtr.limit);
    for (int i = 0; i <= gdtr.limit / 8; i++) {
        printf("i: %d. val: %x\n", i, gdt_tmp[i]);
    }
}

#ifdef TEST
// tests
static void test_basic_parsing() {
    const uint64_t TEST_SEGMENT = 0x120D00345678BEEF;
    assert(gdt_parse_limit(TEST_SEGMENT) == 0xdbeef, "Test failed");
    assert(gdt_parse_base(TEST_SEGMENT) == 0x12345678, "Test failed");
}

#define SEG_DESCTYPE(x) ((x) << 0x04)       // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x) ((x) << 0x07)           // Present
#define SEG_SAVL(x) ((x) << 0x0C)           // Available for system use
#define SEG_LONG(x) ((x) << 0x0D)           // Long mode
#define SEG_SIZE(x) ((x) << 0x0E)           // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x) ((x) << 0x0F)           // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x) (((x) & 0x03) << 0x05)  // Set privilege level (0 - 3)

#define SEG_DATA_RD 0x00         // Read-Only
#define SEG_DATA_RDA 0x01        // Read-Only, accessed
#define SEG_DATA_RDWR 0x02       // Read/Write
#define SEG_DATA_RDWRA 0x03      // Read/Write, accessed
#define SEG_DATA_RDEXPD 0x04     // Read-Only, expand-down
#define SEG_DATA_RDEXPDA 0x05    // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD 0x06   // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07  // Read/Write, expand-down, accessed
#define SEG_CODE_EX 0x08         // Execute-Only
#define SEG_CODE_EXA 0x09        // Execute-Only, accessed
#define SEG_CODE_EXRD 0x0A       // Execute/Read
#define SEG_CODE_EXRDA 0x0B      // Execute/Read, accessed
#define SEG_CODE_EXC 0x0C        // Execute-Only, conforming
#define SEG_CODE_EXCA 0x0D       // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC 0x0E      // Execute/Read, conforming
#define SEG_CODE_EXRDCA 0x0F     // Execute/Read, conforming, accessed

#define GDT_CODE_PL0                                                                        \
    SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | \
        SEG_PRIV(0) | SEG_CODE_EXRD

#define GDT_DATA_PL0                                                                        \
    SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | \
        SEG_PRIV(0) | SEG_DATA_RDWR

#define GDT_CODE_PL3                                                                        \
    SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | \
        SEG_PRIV(3) | SEG_CODE_EXRD

#define GDT_DATA_PL3                                                                        \
    SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | \
        SEG_PRIV(3) | SEG_DATA_RDWR

static void test_create_descriptor() {
    uint64_t ret = 0;
    ret = create_descriptor(0, 0, 0);
    assert(ret == 0, "Failed test 1");
    ret = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL0));
    printf("Expected: %x, Got: %x", 0x00CF9A000000FFFF, ret);
    assert(ret == 0x00CF9A000000FFFF, "Failed test 2");
    ret = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL0));
    assert(ret == 0x00CF92000000FFFF, "Failed test 3");
    ret = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL3));
    assert(ret == 0x00CFFA000000FFFF, "Failed test 3");
    ret = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL3));
    assert(ret == 0x00CFF2000000FFFF, "Failed test 4");
}

static void test_insert_base() {
    uint32_t base = 0x0;

    uint64_t ret = insert_base(base);
    assert(ret == 0, "test_insert_base failed");
}

static void test_insert_limit() {
    uint32_t limit = 0x000FFFFF;

    uint64_t ret = insert_limit(limit);
    uint64_t exp = 0x000F00000000FFFF;

    assert(ret == exp, "test_insert_limit FAILED");
}

static void test_get_binary_from_access_byte() {
    struct AccessByte code_access_byte;
    set_p(&code_access_byte, 1);
    set_dpl(&code_access_byte, 0);
    set_s(&code_access_byte, 1);
    set_e(&code_access_byte, 1);
    set_dc(&code_access_byte, 0);
    set_rw(&code_access_byte, 1);
    set_a(&code_access_byte, 0);

    uint8_t ret = get_binary_from_access_byte(code_access_byte);
    assert(ret == 0x9A, "test_get_binary_from_access_byte FAILED");
}

void run_gdt_tests() {
    test_basic_parsing();
    test_create_descriptor();
    test_insert_base();
    test_insert_limit();
    test_get_binary_from_access_byte();
    printf("Global Descriptor Table: [OK]\n");
}
#endif
