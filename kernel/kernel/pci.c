#include <kernel/panic.h>
#include <kernel/pci.h>
#include <utils.h>

uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    assert((offset & 0x3) == 0 && offset <= 0xFC, "offset must be 4-byte aligned");

    uint32_t address = ((uint32_t)1 << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | ((uint32_t)(offset & 0xFC));

    outl(0xCF8, address);
    return inl(0xCFC);
}

uint32_t pci_read_register(PciAddress address, uint8_t reg) {
    return pci_config_read(address.bus, address.slot, 0, reg * 4);
}

Pci find_pci_address(uint16_t vendor_id, uint16_t device_id) {
    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            uint32_t val = pci_config_read(bus, slot, 0, 0x00);
            uint16_t vendor = val & 0xFFFF;

            if (vendor != 0xFFFF) {
                uint16_t device = (val >> 16) & 0xFFFF;

                // LOG("val: 0x%x", val);
                // LOG("vendor: 0x%x, device: 0x%x", vendor, device);

                if (vendor == vendor_id && device == device_id) {
                    // now look up the header type
                    PciAddress pciAddress = {.bus = bus, .slot = slot};

                    uint32_t header_type = (pci_read_register(pciAddress, 0x03) >> 16) & 0xFFFF;
                    PciHeader pci_header;

                    if (header_type == 0)
                        pci_header = General;
                    else if (header_type == 0x2)
                        pci_header = PciToPci;
                    else if (header_type == 0x3)
                        pci_header = PciToCardBridge;
                    else
                        panic("header type could not be identified for pci device");

                    Pci pci = {.address = pciAddress, .header_type = pci_header};

                    return pci;
                }
            }
        }
    }

    panic("Could not find device");
}

uint32_t find_io_base(Pci pci) {
    assert(pci.header_type == General, "find_io_base cannot be used for non General pci devices");
    for (int i = 0; i <= 5; i++) {
        uint32_t base_address = pci_read_register(pci.address, 0x04 + i);
        if ((base_address & 1) && base_address > 0) {
            return (base_address & ~0b11);
        }
    }
    panic("Could not find base address");
}

uint32_t find_mmap_base(Pci pci) {
    assert(pci.header_type == General, "find_io_base cannot be used for non General pci devices");
    for (int i = 0; i <= 5; i++) {
        uint32_t base_address = pci_read_register(pci.address, 0x04 + i);
        if ((base_address & 1) == 0 && base_address > 0) {
            return (base_address & ~0b1111);
        }
    }
    panic("Could not find base address");
}

#ifdef TEST
void test_rtl_driver() {
    Pci pci = find_pci_address(0x10EC, 0x8139);
    uint32_t base_addr = find_io_base(pci);
    uint8_t mac_1[6];

    uint32_t mac_first = inl(base_addr);
    uint32_t mac_sec = inl(base_addr + 4);

    mac_1[0] = mac_first & 0xFF;
    mac_1[1] = (mac_first >> 8) & 0xFF;
    mac_1[2] = (mac_first >> 16) & 0xFF;
    mac_1[3] = (mac_first >> 24) & 0xFF;
    mac_1[4] = (mac_sec) & 0xFF;
    mac_1[5] = (mac_sec >> 8) & 0xFF;

    uint32_t memory_addr = find_mmap_base(pci);

    uint8_t* mmio = (uint8_t*)memory_addr;
    uint8_t mac[6];
    for (int i = 0; i < 6; i++) mac[i] = mmio[i];
    // LOG("MAC from IO: %x:%x:%x:%x:%x:%x",
    // mac_1[0], mac_1[1], mac_1[2], mac_1[3], mac_1[4], mac_1[5]);
    // LOG("MAC from MMIO: %x:%x:%x:%x:%x:%x",
    // mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    for (int i = 0; i < 6; i++) {
        assert(mac_1[i] == mac[i], "mac address mismatch between io and memory retried");
    }
}
void run_pci_tests() {
    test_rtl_driver();
    LOG_GREEN("PCI: [OK]");
}
#endif
