#ifndef __ALLOCATOR__
#define __ALLOCATOR__

#include<stddef.h>
#include<kernel/multiboot.h>
#include<stdint.h>

struct FreeSegment {
    uint32_t size;
    struct FreeSegment* next_segment;
}__attribute__((packed));

void initialize_free_segments(multiboot_info_t* mbd);

#endif
