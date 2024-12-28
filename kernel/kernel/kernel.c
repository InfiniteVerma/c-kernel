#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/panic.h>
#include <kernel/allocator.h>
#include <kernel/io/uart.h>
#include <string.h>

extern unsigned int get_esp();

#define DEBUG

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {

    //unsigned int esp = get_esp();
	terminal_initialize();
    assert(init_serial() == 0, "Could not initialize serial port");
    //printf("Stack pointer: 0x%x\n", esp);
	printf("Hello, kernel World, bootloader: %s\n", mbd->boot_loader_name);
    
#ifdef DEBUG
    parse_multiboot_info(mbd, magic);
#endif
    initialize_free_segments(mbd);

    int* allocated_ptr = (int*)malloc(4 * sizeof(int));
    allocated_ptr[0] = 1;
    allocated_ptr[1] = 2;
    allocated_ptr[2] = 3;
    allocated_ptr[3] = 4;

    for(int i=0;i<4;i++) {
        printf("%d\n", allocated_ptr[i]);
    }

    char* str_array = (char*)malloc(11 * sizeof(char));
    memset(str_array, '\0', 11);
    memcpy(str_array, "helloworld\0", 11);
    printf("<%s>\n", str_array);

    free(allocated_ptr);
    free(str_array);
    
    printf("DONE\n");
    //exit_(0);
}
