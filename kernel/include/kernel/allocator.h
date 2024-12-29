#ifndef __ALLOCATOR__
#define __ALLOCATOR__

#include<stddef.h>
#include<kernel/multiboot.h>
#include<stdint.h>

struct FreeSegment {
    uint32_t size;
    struct FreeSegment* next_segment;
}__attribute__((packed));

struct AllocatedSegment {
    uint32_t size;
    struct AllocatedSegment* next_segment;
}__attribute__((packed));

void initialize_free_segments(multiboot_info_t* mbd);

void* malloc(size_t); // TODO move to libc?
void free(void* ptr); // TODO move to libc?

int FreeSegment_equals(const struct FreeSegment* a, const struct FreeSegment* b);
void FreeSegment_print(const struct FreeSegment*);
void FreeSegment_delete(const struct FreeSegment*);

// TODO make this usable by any all linked lists?
struct FreeSegment* deep_copy(const struct FreeSegment* a);

#ifdef TEST
void run_allocator_tests();
#endif

#endif /* __ALLOCATOR__ */
