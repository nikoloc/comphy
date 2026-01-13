#ifndef ALLOC_H
#define ALLOC_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static inline void *
alloc(size_t size) {
    void *p = malloc(size);
    assert(p);

    memset(p, 0, size);
    return p;
}

#endif
