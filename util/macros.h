#ifndef MACROS_H
#define MACROS_H

#include <math.h>
#include <stddef.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(v, a, b) (MAX(MIN((v), (b)), (a)))

#define FEQUAL(f, v) (fabsf((f) - (v)) < 1e-6)

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

#define UNUSED(x) ((void)(x))
#define TODO(x) (ASSERT(0 && x))
#define UNREACHABLE() (__builtin_unreachable())

#define CONTAINER_OF(ptr, type, member) (type *)((char *)(ptr) - offsetof(type, member))
#define WITH_DEFAULT(value, default_value) ((value) ? (value) : (default_value))

#endif
