#include <kernel/io/uart.h>

#define PORT 0x3f8  // COM1

void outb(uint16_t port, uint8_t byte) {
    asm volatile("outb %0, %1" : : "a"(byte), "d"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
    return ret;
}

int init_serial() {
    outb(PORT + 1, 0x00);  // Disable all interrupts
    outb(PORT + 3, 0x80);  // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03);  // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00);  //                  (hi byte)
    outb(PORT + 3, 0x03);  // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B);  // IRQs enabled, RTS/DSR set
    outb(PORT + 4, 0x1E);  // Set in loopback mode, test the serial chip
    outb(PORT + 0,
         0xAE);  // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(PORT + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT + 4, 0x0F);
    return 0;
}

void serial_write(const char* msg, int len) {
    for (int i = 0; i < len; i++) {
        outb(PORT, msg[i]);
    }
    outb(PORT, 'd');
}

void serial_putchar(const char c) {
    outb(PORT, c);
}

void exit_(const uint8_t code) {
    const uint8_t ISA_DEBUG_EXIT_PORT = 0xf4;
    outb(ISA_DEBUG_EXIT_PORT, code);
}
