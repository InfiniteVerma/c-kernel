#include <kernel/gdt.h>
#include <kernel/panic.h>
#ifdef TEST
#include <stdio.h>
#endif

#define FLAGS 1100

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

    ret |= access_byte.a;
    ret |= (access_byte.rw << 1);
    ret |= (access_byte.dc << 2);
    ret |= (access_byte.e << 3);
    ret |= (access_byte.s << 4);
    ret |= (access_byte.dpl << 5);
    ret |= (access_byte.p < 7);

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

static const uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag) {
    uint64_t ret = 0;
    // first top 32 bits
    ret |= (base >> 16) & 0x000000FF;
    ret |= limit & 0x000F0000;
    ret |= base & 0xFF000000;
    ret |= (flag << 8) & 0x00F0FF00;

    ret <<= 32;

    ret |= (base >> 16);
    ret |= limit & 0x0000FFFF;

    return ret;
}

static void fill_gdt_vals(uint64_t* segments) {
    struct AccessByte code_access_byte;
    set_p(&code_access_byte, 1);
    set_dpl(&code_access_byte, 0);
    set_s(&code_access_byte, 1);
    set_e(&code_access_byte, 1);
    set_dc(&code_access_byte, 0);
    set_rw(&code_access_byte, 0);
    set_a(&code_access_byte, 1);

    uint16_t flags = 0x1100;
    flags = (flags << 12);
    flags |= get_binary_from_access_byte(code_access_byte);

    const uint64_t code_segment = create_descriptor(0, 0xffffffff, flags);

    struct AccessByte data_access_byte;
    set_p(&data_access_byte, 1);
    set_dpl(&data_access_byte, 0);
    set_s(&data_access_byte, 1);
    set_e(&data_access_byte, 0);
    set_dc(&data_access_byte, 0);
    set_rw(&data_access_byte, 1);
    set_a(&data_access_byte, 1);

    flags = 0x1100;
    flags = (flags << 12);
    flags |= get_binary_from_access_byte(data_access_byte);
    const uint64_t data_segment = create_descriptor(0, 0xffffffff, flags);

    const uint64_t null_segment = 0;

    segments[0] = null_segment;
    segments[1] = code_segment;
    segments[2] = data_segment;
}

void init_gdt() {
    uint64_t gdt[3] = {0};
    fill_gdt_vals(gdt);

    GDTDescriptor gdtr = {
        .limit = sizeof(gdt) - 1,
        .base = (uint64_t)gdt,
    };

    asm volatile (
        "lgdt (%0)"
        :  // No output operands
        : "r" (&gdtr) : "memory"
    );
}

#ifdef TEST
// tests
static void test_basic_parsing() {
    const uint64_t TEST_SEGMENT = 0x120D00345678BEEF;
    assert(gdt_parse_limit(TEST_SEGMENT) == 0xdbeef, "Test failed");
    assert(gdt_parse_base(TEST_SEGMENT) == 0x12345678, "Test failed");
}

#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
#define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)
 
#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A // Execute/Read
#define SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed
 
#define GDT_CODE_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_DATA_RDWR
 
#define GDT_CODE_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_DATA_RDWR

static void test_create_descriptor() {
    uint64_t ret = 0;
    ret = create_descriptor(0, 0, 0);
    assert(ret == 0, "Failed test 1");
    ret = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL0));
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

void run_gdt_tests() {
    test_basic_parsing();
    test_create_descriptor();
    test_insert_base();
    test_insert_limit();
    printf("Global Descriptor Table: [OK]\n");
}
#endif
