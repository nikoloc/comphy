#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ASSERT
#ifdef DEBUG
#define ASSERT(expr)                                                                       \
    do {                                                                                   \
        if(!(expr)) {                                                                      \
            fprintf(stderr, "[%s, %d] assertion failed: %s\n", __FILE__, __LINE__, #expr); \
            __builtin_trap();                                                              \
        }                                                                                  \
    } while(0)
#else
#define ASSERT(expr) \
    do {             \
    } while(0)
#endif
#endif

#define COPY(dest, src, count, type) (memcpy(dest, src, count * sizeof(type)))
#define MOVE(dest, src, count, type) (memmove(dest, src, count * sizeof(type)))
#define COMPARE(a, b, count, type) (memcmp(a, b, count * sizeof(type)) == 0)
#define ZERO(dest, count, type) (memset(dest, 0, count * sizeof(type)))

static inline void *
_allocate(size_t count, size_t size) {
    void *ptr = calloc(count, size);
    ASSERT(ptr);

    return ptr;
}

static inline void *
_reallocate(void *ptr, size_t count, size_t size) {
    ptr = realloc(ptr, count * size);
    ASSERT(ptr);

    return ptr;
}

#define ALLOCATE(type) ((type *)_allocate(1, sizeof(type)))
#define ALLOCATE_MANY(count, type) ((type *)_allocate(count, sizeof(type)))
#define REALLOCATE(ptr, count, type) ((type *)_reallocate(ptr, count, sizeof(type)))
// for convinience i also have it by this name, more suitable to use when we are not sure
// wheather the pointer we have already has some memory allocated, or not
#define RESERVE(ptr, count, type) ((type *)_reallocate(ptr, count, sizeof(type)))

#endif
