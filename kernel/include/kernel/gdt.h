#ifndef __GDT__
#define __GDT__

#include <stdint.h>

struct AccessByte {
    uint8_t p;
    uint8_t dpl;
    uint8_t s;
    uint8_t e;
    uint8_t dc;
    uint8_t rw;
    uint8_t a;
};

//void set_p(struct AccessByte*, const uint8_t);
//void set_dpl(struct AccessByte*, const uint8_t);
//void set_s(struct AccessByte*, const uint8_t);
//void set_e(struct AccessByte*, const uint8_t);
//void set_dc(struct AccessByte*, const uint8_t);
//void set_rw(struct AccessByte*, const uint8_t);
//void set_a(struct AccessByte*, const uint8_t);

//const uint8_t get_binary_from_access_byte(const struct AccessByte);

struct Flags {
    uint8_t g;
    uint8_t db;
    uint8_t l;
};

typedef struct {
    uint16_t limit;
    uint16_t base;
} __attribute__((packed)) GDTDescriptor;

//static const uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag);

void init_gdt();

#ifdef TEST
void run_gdt_tests();
#endif

#endif /* __GDT__ */
