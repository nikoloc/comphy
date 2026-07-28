#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FNV_PRIME 16777619u
#define FNV_OFFSET 2166136261u

static inline uint32_t
_hash(char *key) {
    uint32_t hash = FNV_OFFSET;
    for(size_t i = 0; key[i] != 0; i++) {
        hash ^= (uint32_t)key[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

#define DEFINE_HASHMAP(type, prefix)                                               \
    typedef struct prefix##_entry {                                                \
        char *key;                                                                 \
        type value;                                                                \
    } prefix##_entry_t;                                                            \
                                                                                   \
    typedef struct prefix {                                                        \
        prefix##_entry_t *entries;                                                 \
        size_t cap;                                                                \
        size_t count;                                                              \
    } prefix##_t;                                                                  \
                                                                                   \
    static inline void prefix##_init(prefix##_t *h) {                              \
        memset(h, 0, sizeof(*h));                                                  \
    }                                                                              \
                                                                                   \
    static inline void prefix##_deinit(prefix##_t *h) {                            \
        for(size_t i = 0; i < h->cap; i++) {                                       \
            if(h->entries[i].key) {                                                \
                free(h->entries[i].key);                                           \
            }                                                                      \
        }                                                                          \
                                                                                   \
        free(h->entries);                                                          \
    }                                                                              \
                                                                                   \
    static void prefix##_rehash(prefix##_t *h, size_t new_cap) {                   \
        prefix##_entry_t *old_entries = h->entries;                                \
        size_t old_cap = h->cap;                                                   \
                                                                                   \
        h->entries = calloc(new_cap, sizeof(struct prefix##_entry));               \
        h->cap = new_cap;                                                          \
        h->count = 0;                                                              \
                                                                                   \
        for(size_t i = 0; i < old_cap; i++) {                                      \
            if(old_entries[i].key) {                                               \
                uint32_t hashed = _hash(old_entries[i].key);                       \
                size_t index = hashed % h->cap;                                    \
                                                                                   \
                while(h->entries[index].key) {                                     \
                    index = (index + 1) % h->cap;                                  \
                }                                                                  \
                                                                                   \
                h->entries[index].key = old_entries[i].key;                        \
                h->entries[index].value = old_entries[i].value;                    \
                h->count++;                                                        \
            }                                                                      \
        }                                                                          \
                                                                                   \
        free(old_entries);                                                         \
    }                                                                              \
                                                                                   \
    static inline void prefix##_set(prefix##_t *h, char *key, type value) {        \
        if(h->cap == 0) {                                                          \
            prefix##_rehash(h, 16);                                                \
        } else if(h->count >= h->cap / 2) {                                        \
            prefix##_rehash(h, h->cap * 2);                                        \
        }                                                                          \
                                                                                   \
        uint32_t hashed = _hash(key);                                              \
        size_t index = hashed % h->cap;                                            \
        size_t probes = 0;                                                         \
                                                                                   \
        while(probes < h->cap) {                                                   \
            if(h->entries[index].key && strcmp(h->entries[index].key, key) == 0) { \
                h->entries[index].value = value;                                   \
                return;                                                            \
            }                                                                      \
            if(!h->entries[index].key) {                                           \
                h->entries[index].key = strdup(key);                               \
                h->entries[index].value = value;                                   \
                h->count++;                                                        \
                return;                                                            \
            }                                                                      \
            index = (index + 1) % h->cap;                                          \
            probes++;                                                              \
        }                                                                          \
    }                                                                              \
                                                                                   \
    static inline bool prefix##_get(prefix##_t *h, char *key, type *dest) {        \
        if(h->cap == 0) {                                                          \
            return false;                                                          \
        }                                                                          \
                                                                                   \
        uint32_t hashed = _hash(key);                                              \
        size_t index = hashed % h->cap;                                            \
        size_t probes = 0;                                                         \
                                                                                   \
        while(probes < h->cap) {                                                   \
            if(h->entries[index].key && strcmp(h->entries[index].key, key) == 0) { \
                *dest = h->entries[index].value;                                   \
                return true;                                                       \
            }                                                                      \
                                                                                   \
            index = (index + 1) % h->cap;                                          \
            probes++;                                                              \
        }                                                                          \
                                                                                   \
        return false;                                                              \
    }                                                                              \
                                                                                   \
    static inline bool prefix##_remove(prefix##_t *h, char *key) {                 \
        if(h->cap == 0) {                                                          \
            return false;                                                          \
        }                                                                          \
                                                                                   \
        uint32_t hashed = _hash(key);                                              \
        size_t index = hashed % h->cap;                                            \
        size_t probes = 0;                                                         \
                                                                                   \
        while(probes < h->cap) {                                                   \
            if(h->entries[index].key && strcmp(h->entries[index].key, key) == 0) { \
                free(h->entries[index].key);                                       \
                h->entries[index].key = NULL;                                      \
                h->count--;                                                        \
                return true;                                                       \
            }                                                                      \
                                                                                   \
            index = (index + 1) % h->cap;                                          \
            probes++;                                                              \
        }                                                                          \
                                                                                   \
        return false;                                                              \
    }                                                                              \
                                                                                   \
    static inline bool prefix##_has(prefix##_t *h, char *key) {                    \
        type dummy;                                                                \
        return prefix##_get(h, key, &dummy);                                       \
    }                                                                              \
    struct prefix

// this thing claude wrote for me, it uses some shinanigans to get the macro to look nice when using it
#define hashmap_for_each(prefix, map_ptr, key_var, value_var)                                                     \
    for(size_t _i = 0; _i < (map_ptr)->cap; _i++)                                                                 \
        if((map_ptr)->entries[_i].key)                                                                            \
            for(char *key_var = (map_ptr)->entries[_i].key, *_k = key_var; _k; _k = NULL)                         \
                for(typeof((map_ptr)->entries[0].value) value_var = (map_ptr)->entries[_i].value, _v = value_var; \
                        (void)_v, _k; _k = NULL)

#endif
