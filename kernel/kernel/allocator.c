#include<kernel/multiboot.h>
#include<kernel/allocator.h>
#include<kernel/panic.h>
#include<string.h>
#include<stdio.h>
#include<stdint.h>

struct FreeSegment* freeSegment = NULL;
extern unsigned long KERNEL_START;
extern unsigned long KERNEL_END;

void initialize_free_segments(multiboot_info_t* mbd) {
    printf("initialize_free_segments START\n");

    assert(mbd != NULL, "mbd is NULL");
    assert(freeSegment == NULL, "freeSegment is already initialized");

    struct multiboot_mmap_entry entry;
    int found_big_block = 0;
    for(int i = 0; i < mbd->mmap_length;
            i += sizeof(multiboot_memory_map_t))
    {
        multiboot_memory_map_t* mmmt = 
            (multiboot_memory_map_t*) (mbd->mmap_addr + i);

        if(mmmt->addr == &KERNEL_START) {
            printf("FOUND MATCHING: Size: %u : Addr: 0x%x : Len %lluK : Type %u\n", mmmt->size, mmmt->addr, mmmt->len / 1024, mmmt->type);
            entry.size = mmmt->size;
            entry.addr = mmmt->addr;
            entry.len = mmmt->len;
            entry.type = mmmt->type;
            found_big_block = 1;
            break;
        }
    }

    assert(found_big_block, "Could not find a big block of memory!");

    unsigned int reserved_mem_length = &KERNEL_END - &KERNEL_START;
    uint32_t big_block_size = entry.len - reserved_mem_length - sizeof(*freeSegment);

    freeSegment = (struct FreeSegment*)(&KERNEL_END);
    freeSegment->size = big_block_size;
    freeSegment->next_segment = NULL;
}
