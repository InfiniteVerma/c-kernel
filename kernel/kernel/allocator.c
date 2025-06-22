#include <kernel/allocator.h>
#include <kernel/multiboot.h>
#include <kernel/panic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

struct FreeSegment* freeSegment = NULL;
extern unsigned long KERNEL_START;
extern unsigned long KERNEL_END;

#define ALIGN 8

static void insert_segment_into_free_list(struct FreeSegment*);
static void merge_segments(struct FreeSegment*, struct FreeSegment*);

void initialize_free_segments(multiboot_info_t* mbd) {
#ifdef DEBUG
    LOG("initialize_free_segments START");
#endif

    assert(sizeof(struct FreeSegment) == sizeof(struct AllocatedSegment),
           "FreeSegment and AllocatedSegment struct sizes are different!");

    assert(mbd != NULL, "mbd is NULL");
    assert(freeSegment == NULL, "freeSegment is already initialized");

    struct multiboot_mmap_entry entry;
    int found_big_block = 0;
    for (int i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*)(mbd->mmap_addr + i);

        if ((uintptr_t)mmmt->addr == (uintptr_t)&KERNEL_START) {
#ifdef DEBUG
            LOG("FOUND MATCHING: Size: %u : Addr: 0x%x : Len %lluK : Type %u", mmmt->size,
                mmmt->addr, mmmt->len / 1024, mmmt->type);
#endif
            entry.size = mmmt->size;
            entry.addr = mmmt->addr;
            entry.len = mmmt->len;
            entry.type = mmmt->type;
            found_big_block = 1;
            break;
        }
    }

    assert(found_big_block, "Could not find a big block of memory!");

    uintptr_t reserved_mem_length = (uintptr_t)&KERNEL_END - (uintptr_t)&KERNEL_START;
    uint32_t big_block_size = entry.len - reserved_mem_length - sizeof(struct FreeSegment);

    freeSegment = (struct FreeSegment*)(&KERNEL_END);
    freeSegment->size = big_block_size;
    freeSegment->next_segment = NULL;
#ifdef DEBUG
    LOG("Free memory: %d", freeSegment->size);
#endif
}

void* malloc(size_t size) {
    struct FreeSegment* free_segment_ptr = freeSegment;

    while (free_segment_ptr) {
        uintptr_t start_ptr = (uintptr_t)free_segment_ptr;
        uintptr_t end_ptr =
            (uintptr_t)free_segment_ptr + free_segment_ptr->size + sizeof(struct FreeSegment);

        uintptr_t ptr = end_ptr - size;
        ptr = ptr - (ptr % ALIGN);
        ptr = ptr - sizeof(struct AllocatedSegment);  // Using this ptr as it's same size. We're
                                                      // adding size of one AllocatedSegment*

        if (ptr < start_ptr + sizeof(struct FreeSegment)) {
            free_segment_ptr = free_segment_ptr->next_segment;
            continue;
        } else {
            // Store size allocated in AllocatedSegment's meta data
            // size == requested size + padded bytes
            struct AllocatedSegment* header_ptr = (struct AllocatedSegment*)ptr;
            header_ptr->size =
                (uintptr_t)end_ptr - (uintptr_t)header_ptr - sizeof(struct AllocatedSegment);
            header_ptr->next_segment = NULL;  // BUFFER

            // Update free segment's size
            uint32_t new_free_size = free_segment_ptr->size - (end_ptr - ptr);
            free_segment_ptr->size = new_free_size;
            return (void*)(ptr + sizeof(struct AllocatedSegment));
        }
    }

    panic("Could not allocator memory!");
    return NULL;
}

/*
 * Input: ptr
 *
 *  - Move back to get AllocatedSegment*
 *  - Convert it into a FreeSegment*
 *  - Merge it with FreeSegment list
 */
void free(void* ptr) {
    // move back to get AllocatedSegment info
    struct AllocatedSegment* segment_to_free =
        (struct AllocatedSegment*)(ptr - sizeof(struct AllocatedSegment));

    uint32_t size = segment_to_free->size;

    // convert it to FreeSegment*
    struct FreeSegment* new_free_segment = (struct FreeSegment*)segment_to_free;
    new_free_segment->size = size;
    new_free_segment->next_segment = NULL;

    insert_segment_into_free_list(new_free_segment);
}

static void insert_segment_into_free_list(struct FreeSegment* new_free_segment) {
    struct FreeSegment* free_list_ptr = freeSegment;

    while (free_list_ptr) {
        if ((uintptr_t)(free_list_ptr->next_segment) > (uintptr_t)new_free_segment ||
            free_list_ptr->next_segment == NULL) {
            struct FreeSegment* old_ptr = free_list_ptr->next_segment;
            free_list_ptr->next_segment = new_free_segment;
            new_free_segment->next_segment = old_ptr;

            merge_segments(new_free_segment, new_free_segment->next_segment);
            merge_segments(free_list_ptr, new_free_segment);
            return;
        }

        free_list_ptr = free_list_ptr->next_segment;
    }

    panic("Could not deallocate!");
}

static void merge_segments(struct FreeSegment* a, struct FreeSegment* b) {
    if ((uintptr_t)a + sizeof(struct FreeSegment) + (uintptr_t)a->size == (uintptr_t)b) {
        a->size = a->size + sizeof(struct FreeSegment) + b->size;
        a->next_segment = b->next_segment;
    }
}

int FreeSegment_equals(const struct FreeSegment* a, const struct FreeSegment* b) {
    if (a == NULL && b == NULL) return 1;
    if (a == NULL || b == NULL) return 0;

    return a->size == b->size && FreeSegment_equals(a->next_segment, b->next_segment);
}

void FreeSegment_print(const struct FreeSegment* a) {
    LOG("FreeSegment: ");
    if (!a) {
        LOG("NULL");
        return;
    }
    LOG("Size: %d", a->size);
}

// TODO make this usable by any all linked lists?
struct FreeSegment* deep_copy(const struct FreeSegment* a) {
    if (!a) {
        return NULL;
    }

    struct FreeSegment* free_segment_copy = (struct FreeSegment*)malloc(sizeof(struct FreeSegment));
    free_segment_copy->size = a->size;  // RHS size is modified by above malloc
    free_segment_copy->next_segment = deep_copy(a->next_segment);
    return free_segment_copy;
}

void FreeSegment_delete(const struct FreeSegment* a) {
    if (!a) return;

    FreeSegment_delete(a->next_segment);
    free((void*)a);
}

#ifdef TEST
static void test_1() {
    extern struct FreeSegment* freeSegment;
    struct FreeSegment* before = deep_copy(freeSegment);
    int* allocated_ptr = (int*)malloc(4 * sizeof(int));
    allocated_ptr[0] = 1;
    allocated_ptr[1] = 2;
    allocated_ptr[2] = 3;
    allocated_ptr[3] = 4;

    char* str_array = (char*)malloc(11 * sizeof(char));
    memset(str_array, '\0', 11);
    memcpy(str_array, "helloworld\0", 11);

    free(allocated_ptr);
    free(str_array);

    assert(FreeSegment_equals(before, freeSegment), "Does not match");
    FreeSegment_delete(before);
}

void run_allocator_tests() {
    test_1();
    LOG_GREEN("Allocator: [OK]");
}
#endif
