#ifndef __KHEAP__H__
#define __KHEAP__H__


#include <stdint.h>


// Heap virtual base
#define KHEAP_BASE 0xFFFFFF8000000000ULL

// Alignment for all returned pointers (16 bytes for any SSE types TYPE)
#define KHEAP_ALIGNMENT 16

// Canary value for detecting heap overflows (magic number)
#define KHEAP_CANARY_HEAD 0xDEADBEEFCAFEBABEULL
#define KHEAP_CANARY_TAIL 0xCAFEBABEDEADBEEFULL


// Initialize the kernel heap. Must be called before any calls to kheap_alloc or kheap_free
void kheap_init(void);

// Allocate a block of memory of at least 'size' bytes and return a pointer to it.
// The returned pointer is guaranteed to be aligned to KHEAP_ALIGNMENT. returns NULL if allocation fails.
void *kmalloc(uint64_t size);

// Allocate and zero-initialize an array of 'count' elements of 'size' bytes each. Returns NULL if allocation fails.
void *kcalloc(uint64_t count, uint64_t size);

// Resize an allocation to 'new_size' bytes. If the new size is larger, the new memory is not initialized. Returns pointer to resized block, or NULL if reallocation fails (original block is left unchanged if this happens).
void *krealloc(void *ptr, uint64_t new_size);

// Free a block of memory allocated by kheap_alloc, kcalloc, or krealloc. ptr must be a pointer returned by one of those functions and not already freed. If ptr is NULL, no operation is performed.
void kfree(void *ptr);

// Print stats: total bytes, used, free, largest free block, block count
void kheap_stats(void);

// Walk the heap and validate canary, panics on corruption
void kheap_check(void);

#endif /* __KHEAP__H__ */