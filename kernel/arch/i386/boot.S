# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve a stack for the initial thread.
.section .bss
.align 16
stack_bottom:
.skip 16380 # 16 KiB
stack_top:
.skip 4

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	movl $stack_top, %esp

	# Call the global constructors.
	#call _init

    push %eax
    push %ebx
	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b

.global get_esp
.type get_esp, @function
get_esp:
    mov %esp,%eax

.size _start, . - _start
