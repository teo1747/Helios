#include "pci.h"
#include "../include/io.h"
#include "../include/kprintf.h"
#include "../drivers/serial.h"
#include <stdint.h>


// Built the CONFIG_ADDRESS and select the register to read/write
static uint32_t pci_config_address(uint8_t bus, uint8_t device, uint8_t fonction, uint8_t offset) {
    return(uint32_t)(
        (1U << 31)                     // Enable bit
        | ((uint32_t)bus << 16)        // Bus number
        | ((uint32_t)device << 11)      // Device number
        | ((uint32_t)fonction << 8)     // Function number
        | ((uint32_t)(offset & 0xfc))   // dword-aligned offset
    );
}


// Read a 32-bit value from the PCI configuration space
uint32_t pci_read32(uint8_t bus, uint8_t device, uint8_t fonction, uint8_t offset) {
    outl_func:;  // Placeholder label removed below
    uint32_t address = pci_config_address(bus, device, fonction, offset);
    // We need outl/inl (32 bits port I/O). 

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}


uint16_t pci_read16(uint8_t bus, uint8_t device, uint8_t fonction, uint8_t offset) {
    uint32_t value = pci_read32(bus, device, fonction, offset & 0xFC);
    // Extract the 16-bit value from the 32-bit value low bits
    return (uint16_t)((value >> ((offset & 2) * 8)) & 0xFFFF);

}

uint8_t pci_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset){
    uint32_t value = pci_read32(bus, device, function, offset & 0xFC);
    // Extract the 8-bit value from the 32-bit value low bits
    return (uint8_t)((value >> ((offset & 3) * 8)) & 0xFF);
}

void pci_write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value){
    uint32_t address = pci_config_address(bus, device, function, offset);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}


static struct pci_device devices[PCI_MAX_DEVICES];
static uint32_t pci_device_count = 0;


// Human-readable PCI device description
static const char *pci_device_description(uint8_t class_code) { 
    switch (class_code) {
        case 0x00: return "Unclassified device";
        case 0x01: return "Mass storage controller";
        case 0x02: return "Network controller";
        case 0x03: return "Display controller";
        case 0x04: return "Multimedia controller";
        case 0x05: return "Memory controller";
        case 0x06: return "Bridge device";
        case 0x07: return "Simple communication controller";
        case 0x08: return "Base system peripherals";
        case 0x09: return "Input devices";
        case 0x0A: return "Docking stations";
        case 0x0B: return "Processor/memory/communication controller";
        case 0x0C: return "Serial bus controller";
        case 0x0D: return "Wireless controller";
        case 0x0E: return "Intelligent I/O controller";
        case 0x0F: return "Satellite communication controller";
        case 0x10: return "Encryption/decryption controller";
        case 0x11: return "Data acquisition and signal processing controller";
        default: return "Unknown device";
    }
}

// Probe one bus/device/function and add it to the list of PCI devices. Return true if a device was found.
static bool pci_probe_device(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = pci_read16(bus, device, function, PCI_VENDOR_ID);
    if (vendor_id == 0xFFFF) {
        // Device not present
        return false;
    }
    if (pci_device_count >= PCI_MAX_DEVICES) {
        return true;
    
    }

    // Add the device to the list
    struct pci_device *dev = &devices[pci_device_count++];
    dev->bus = bus;
    dev->device = device;
    dev->function = function;
    dev->vendor_id = vendor_id;
    dev->device_id = pci_read16(bus, device, function, PCI_DEVICE_ID);
    dev->class_code = pci_read8(bus, device, function, PCI_CLASS);
    dev->subclass = pci_read8(bus, device, function, PCI_SUBCLASS);
    dev->prog_if = pci_read8(bus, device, function, PCI_PROG_IF);
    dev->header_type = pci_read8(bus, device, function, PCI_HEADER_TYPE);

    // Print a message about the device
    kprintf("PCI %x:%x.%x %x:%x [%x.%x.%x] %s\n",
                (unsigned int)bus, (unsigned int)device, (unsigned int)function,
                (unsigned int)vendor_id, (unsigned int)dev->device_id,
                (unsigned int)dev->class_code, (unsigned int)dev->subclass, (unsigned int)dev->prog_if,
                pci_device_description(dev->class_code));

    
    return true;
}


void pci_init(void) {
    // Probe all PCI buses
    serial_write_string("\n=== PCI enumeration ===\n");
    pci_device_count = 0;
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t device = 0; device < 32; device++) {
            // Probe function 0 first
            if (!pci_probe_device((uint32_t)bus, (uint32_t)device, 0)) {
                    continue;
            }

            // If multifunction (header type bit 7 set), probe fuctions 1-7
            uint8_t header_type = pci_read8((uint8_t)bus, (uint8_t)device, 0, PCI_HEADER_TYPE);
            if (header_type & 0x80) {
                for (uint8_t func =1; func < 8; func++) {
                    pci_probe_device((uint8_t) bus, (uint8_t) device, func);
                }
            
            }
        }
    }
    kprintf("PCI: found %u device(S)\n", (unsigned int)pci_device_count);
}


uint32_t pci_devices_count(void){
    return pci_device_count;
}

const struct pci_device *pci_get_device(uint32_t index) {
    if (index >= pci_device_count) {
        return NULL;
    }
    return &devices[index];
}