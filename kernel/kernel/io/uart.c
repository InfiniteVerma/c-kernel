#include <kernel/io/uart.h>
#include <stddef.h>
#include <stdio.h>
#include <utils.h>

#include "kernel/future.h"
#include "kernel/interrupts.h"

#define COM1 0x3f8  // COM1

#define DATA_REG COM1
#define INTERRUPT_ENABLE_REG COM1 + 1
#define INTERRUPT_ID_REG COM1 + 2
#define FIFO_CTRL_REG COM1 + 2
#define LINE_CTRL_REG COM1 + 3
#define MODEM_CTRL_REG COM1 + 4
#define LINE_STATUS_REG COM1 + 5
#define MODEM_STATUS_REG COM1 + 6
#define SCRATCH_REG COM1 + 7

static bool is_transmit_ready(void* ctx) {
    if ((inb(LINE_STATUS_REG) & 0x20) == 0) return false;
    return true;
}

void uart_irq() {
    wakeup_executor();
}

int init_serial() {
    outb(INTERRUPT_ENABLE_REG, 0x02);  // Enable transmit register being empty interrupt
    outb(LINE_CTRL_REG, 0x80);         // Enable DLAB (set baud rate divisor)
    outb(DATA_REG, 0x03);              // Set divisor to 3 (lo byte) 38400 baud
    outb(INTERRUPT_ENABLE_REG, 0x00);  //                  (hi byte)
    outb(LINE_CTRL_REG, 0x03);         // 8 bits, no parity, one stop bit
    outb(FIFO_CTRL_REG, 0xC7);         // Enable FIFO, clear them, with 14-byte threshold
    outb(MODEM_CTRL_REG, 0x0B);        // IRQs enabled, RTS/DSR set
    outb(MODEM_CTRL_REG, 0x1E);        // Set in loopback mode, test the serial chip
    outb(DATA_REG,
         0xAE);  // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(DATA_REG) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(MODEM_CTRL_REG, 0x0F);

    INTERRUPT_GUARDED({ register_interrupt(PIC_1_OFFSET + 4, uart_irq); });
    return 0;
}

void serial_putchar(const char c) {
    while (!is_transmit_ready(NULL)) {
    }
    outb(DATA_REG, c);
}

void exit_(const uint8_t code) {
    const uint8_t ISA_DEBUG_EXIT_COM1 = 0xf4;
    outb(ISA_DEBUG_EXIT_COM1, code);
}

Future create_serial_future() {
    Future fut = {.type = IOFuture, .context = NULL, .is_ready = is_transmit_ready};
    return fut;
}

void serial_write(const char* msg, int len) {
    Future serial_fut = create_serial_future();
    for (int i = 0; i < len; i++) {
        await(serial_fut);
        serial_putchar(msg[i]);
    }
}
