#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

// NULL pointer constant
#ifndef NULL
#define NULL ((void *)0)
#endif

// Boolean type (freestanding — no <stdbool.h>)
#ifndef __cplusplus
typedef _Bool bool;
#define true  1
#define false 0
#endif

// size_t — result of sizeof, used for sizes and counts
typedef uint64_t size_t;

// ssize_t — signed size, for functions returning size-or-error
typedef int64_t ssize_t;

#endif /* _TYPES_H */