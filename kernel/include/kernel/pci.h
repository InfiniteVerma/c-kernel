#ifndef __KERNEL_PCI__
#define __KERNEL_PCI__

#include <stdint.h>

struct PciAddress {
    int bus;
    int slot;
};

enum PciHeader { General = 0, PciToPci, PciToCardBridge };

struct Pci {
    struct PciAddress address;
    enum PciHeader header_type;
};

typedef struct PciAddress PciAddress;
typedef struct Pci Pci;
typedef enum PciHeader PciHeader;

uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_read_regiser(PciAddress address, uint8_t reg);

Pci find_pci_address(uint16_t vendor_id, uint16_t device_id);

uint32_t find_io_base(Pci pci);
uint32_t find_mmap_base(Pci pci);

void run_pci_tests();

#endif
