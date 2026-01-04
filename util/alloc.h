#ifndef ALLOC_H
#define ALLOC_H

#include <assert.h>
#include <stdlib.h>

static inline void *
alloc(size_t size) {
    void *p = calloc(1, size);
    assert(p);

    return p;
}

#endif
