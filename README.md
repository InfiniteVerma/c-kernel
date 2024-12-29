## A very ambitious feature list

###  MMU

 - [x] Basic Heap Allocator:
   - [x] Start with a basic heap allocator with a linked list free list
 - [ ] Paging:
   - [ ] Implement virtual memory by introducing basic paging

### Clock/Interrupts
 
 - [x] RTC clock
 - [ ] GDT

### Process Management
 - [ ] PCB
 - [ ] Context switching
 - [ ] Basic process mechanism with a time sliced scheduling

### Multithreading Support
 - [ ] Thread creation/synchronization/thread-local storage

### File System
 - [ ] Minimal FS
    - [ ] Start with a minimal in-memory fs
    - [ ] Implement basic calls like `open`, `read`, `write`, `close`
 - [ ] FAT12 FS

### I/O
 - [ ] Drivers:
    - [x] Console I/O
    - [ ] Disk Driver
 - [ ] Keyboard and Mouse

### IPC
 - [ ] Signals
 - [ ] Pipes

### Networking (TODO enhance list)
 - [ ] Ethernet Driver

### Use Space Programs
 - [ ] Sys call interface
 - [ ] Program loading
 - [ ] Shell

#### Steps to run gdb

./qemu


### References

 - osdev wiki
 - https://youtu.be/gBykJMqDqH0?feature=shared
 - https://www.felixcloutier.com/x86/
