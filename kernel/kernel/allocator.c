#include<kernel/multiboot.h>
#include<kernel/allocator.h>
#include<kernel/panic.h>
#include<string.h>
#include<stdio.h>
#include<stdint.h>

struct FreeSegment* freeSegment = NULL;
extern unsigned long KERNEL_START;
extern unsigned long KERNEL_END;

#define ALIGN 8

static void insert_segment_into_free_list(struct FreeSegment*);
static void merge_segments(struct FreeSegment*, struct FreeSegment*);

void initialize_free_segments(multiboot_info_t* mbd) {
    //printf("initialize_free_segments START\n");

    assert(sizeof(struct FreeSegment) == sizeof(struct AllocatedSegment), "FreeSegment and AllocatedSegment struct sizes are different!");

    assert(mbd != NULL, "mbd is NULL");
    assert(freeSegment == NULL, "freeSegment is already initialized");

    struct multiboot_mmap_entry entry;
    int found_big_block = 0;
    for(int i = 0; i < mbd->mmap_length;
            i += sizeof(multiboot_memory_map_t))
    {
        multiboot_memory_map_t* mmmt = 
            (multiboot_memory_map_t*) (mbd->mmap_addr + i);

        if((uintptr_t)mmmt->addr == (uintptr_t)&KERNEL_START) {
            //printf("FOUND MATCHING: Size: %u : Addr: 0x%x : Len %lluK : Type %u\n", mmmt->size, mmmt->addr, mmmt->len / 1024, mmmt->type);
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
    printf("Free memory: %d\n", freeSegment->size);
#endif
}

void* malloc(size_t size) {
    struct FreeSegment* free_segment_ptr = freeSegment;

    while(free_segment_ptr) {
        uintptr_t start_ptr = (uintptr_t)free_segment_ptr;
        uintptr_t end_ptr = (uintptr_t)free_segment_ptr + free_segment_ptr->size + sizeof(struct FreeSegment);

        uintptr_t ptr = end_ptr - size;
        ptr = ptr - (ptr % ALIGN);
        ptr = ptr - sizeof(struct AllocatedSegment); // Using this ptr as it's same size. We're adding size of one AllocatedSegment*

        if(ptr < start_ptr + sizeof(struct FreeSegment)) {
            free_segment_ptr = free_segment_ptr->next_segment;
            continue;
        } else {

            // Store size allocated in AllocatedSegment's meta data
            // size == requested size + padded bytes
            struct AllocatedSegment* header_ptr = (struct AllocatedSegment*)ptr;
            header_ptr->size = (uintptr_t)end_ptr - (uintptr_t)header_ptr - sizeof(struct AllocatedSegment);
            header_ptr->next_segment = NULL; // BUFFER

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
    struct AllocatedSegment* segment_to_free = (struct AllocatedSegment*)(ptr - sizeof(struct AllocatedSegment));

    uint32_t size = segment_to_free->size;

    // convert it to FreeSegment*
    struct FreeSegment* new_free_segment = (struct FreeSegment*)segment_to_free;
    new_free_segment->size = size;
    new_free_segment->next_segment = NULL;

    insert_segment_into_free_list(new_free_segment);
}

static void insert_segment_into_free_list(struct FreeSegment* new_free_segment) {
    struct FreeSegment* free_list_ptr = freeSegment;

    while(free_list_ptr) {
        if((uintptr_t)(free_list_ptr->next_segment) > (uintptr_t)new_free_segment || free_list_ptr->next_segment == NULL) {
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
    if((uintptr_t)a + sizeof(struct FreeSegment) + (uintptr_t)a->size == (uintptr_t)b) {
        a->size = a->size + sizeof(struct FreeSegment) + b->size;
        a->next_segment = b->next_segment;
    }
}
