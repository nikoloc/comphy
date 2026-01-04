#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include <string.h>

#ifndef ARRAY_DEFAULT_CAP
#define ARRAY_DEFAULT_CAP 16
#endif

#define define_array(type, prefix)                                                                      \
    typedef struct prefix {                                                                             \
        int len, cap;                                                                                   \
        type *data;                                                                                     \
    } prefix##_t;                                                                                       \
                                                                                                        \
    static inline void prefix##_reserve(prefix##_t *array, int cap) {                                   \
        if(cap < ARRAY_DEFAULT_CAP) {                                                                   \
            cap = ARRAY_DEFAULT_CAP;                                                                    \
        }                                                                                               \
                                                                                                        \
        if(cap <= array->cap) {                                                                         \
            return;                                                                                     \
        }                                                                                               \
                                                                                                        \
        array->cap = cap;                                                                               \
        array->data = realloc(array->data, cap * sizeof(type));                                         \
    }                                                                                                   \
                                                                                                        \
    static inline void prefix##_init(prefix##_t *array, int len, type value[len]) {                     \
        if(len == 0) {                                                                                  \
            memset(array, 0, sizeof(*array));                                                           \
            return;                                                                                     \
        }                                                                                               \
                                                                                                        \
        array->len = len;                                                                               \
        prefix##_reserve(array, len);                                                                   \
                                                                                                        \
        memcpy(array->data, value, len * sizeof(type));                                                 \
    }                                                                                                   \
                                                                                                        \
    static inline void prefix##_push(prefix##_t *array, type elem) {                                    \
        if(array->cap == array->len) {                                                                  \
            prefix##_reserve(array, 2 * array->cap);                                                    \
        }                                                                                               \
                                                                                                        \
        array->data[array->len] = elem;                                                                 \
        array->len++;                                                                                   \
    }                                                                                                   \
                                                                                                        \
    static inline void prefix##_remove(prefix##_t *array, int index) {                                  \
        memmove(&array->data[index], &array->data[index + 1], (array->len - index - 1) * sizeof(type)); \
                                                                                                        \
        array->len--;                                                                                   \
    }                                                                                                   \
                                                                                                        \
    static inline void prefix##_remove_fast(prefix##_t *array, int index) {                             \
        array[index] = array[array->len - 1];                                                           \
        array->len--;                                                                                   \
    }                                                                                                   \
                                                                                                        \
    static inline void prefix##_insert(prefix##_t *array, int index, type elem) {                       \
        if(array->cap == array->len) {                                                                  \
            prefix##_reserve(array, 2 * array->cap);                                                    \
        }                                                                                               \
                                                                                                        \
        memmove(&array->data[index + 1], &array->data[index], (array->len - index) * sizeof(type));     \
        array->data[index] = elem;                                                                      \
        array->len++;                                                                                   \
    }                                                                                                   \
                                                                                                        \
    static inline void prefix##_deinit(prefix##_t *array) {                                             \
        if(array->cap > 0) {                                                                            \
            free(array->data);                                                                          \
        }                                                                                               \
    }                                                                                                   \
                                                                                                        \
    static inline type *prefix##_end(prefix##_t *array) {                                               \
        return &array->data[array->len];                                                                \
    }                                                                                                   \
                                                                                                        \
    static inline type *prefix##_last(prefix##_t *array) {                                              \
        return &array->data[array->len - 1];                                                            \
    }                                                                                                   \
                                                                                                        \
    struct prefix

// ^ this last thing is just so we can use ; after the `define_array()`

#endif
