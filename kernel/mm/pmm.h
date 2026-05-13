#ifndef _PMM_H
#define _PMM_H


#include <stdint.h>


// E820 entry as written in stage 2 of the bootloader

struct e820_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi_attr;
} __attribute__((packed)) ;


// Memory map types from Bios

#define E820_USABLE       1
#define E820_RESERVED     2
#define E820_ACPI_RECLAIM 3
#define E820_ACPI_NVS     4
#define E820_BAD_MEMORY   5

// Memory map BUFFER

#define E820_COUNT_ADDR   0X7000
#define E820_ENTRIES_ADDR 0X7004

#define PAGE_SIZE 4096


void pmm_init(void);
void pmm_print_map(void);
void *pmm_alloc_page(void);
void pmm_free_page(void *page);
void pmm_print_stats(void);




#endif // _PMM_H