#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct list {
    struct list *prev;
    struct list *next;
} list_t;

static inline void
list_init(list_t *list) {
    list->prev = list;
    list->next = list;
}

static inline void
list_insert(list_t *list, list_t *elm) {
    elm->prev = list;
    elm->next = list->next;
    list->next = elm;
    elm->next->prev = elm;
}

static inline void
list_remove(list_t *elm) {
    elm->prev->next = elm->next;
    elm->next->prev = elm->prev;
    elm->next = NULL;
    elm->prev = NULL;
}

static inline int
list_length(list_t *list) {
    int count = 0;
    list_t *e = list->next;
    while(e != list) {
        e = e->next;
        count++;
    }

    return count;
}

static inline bool
list_is_empty(list_t *list) {
    return list->next == list;
}

static inline bool
list_is_first(list_t *list, list_t *elm) {
    return list->prev == elm;
}

static inline bool
list_is_last(list_t *list, list_t *elm) {
    return list->next == elm;
}

static inline void
list_insert_list(list_t *list, list_t *other) {
    if(list_is_empty(other))
        return;

    other->next->prev = list;
    other->prev->next = list->next;
    list->next->prev = other->prev;
    list->next = other->next;
}

#ifndef container_of
#define container_of(ptr, type, member) (type *)((char *)(ptr) - offsetof(type, member))
#endif

#define list_for_each(type, it, head, member)                                       \
    for(type *it = container_of((head)->next, type, member); &it->member != (head); \
            it = container_of(it->member.next, type, member))

#define list_for_each_safe(type, it, head, member)                                                                    \
    for(type *it = container_of((head)->next, type, member), *it##_tmp = container_of(it->member.next, type, member); \
            &it->member != (head); it = it##_tmp, it##_tmp = container_of(it->member.next, type, member))

#define list_for_each_reverse(type, it, head, member)                               \
    for(type *it = container_of((head)->prev, type, member); &it->member != (head); \
            it = container_of(it->member.prev, type, member))

#define list_for_each_reverse_safe(type, it, head, member)                                                            \
    for(type *it = container_of((head)->prev, type, member), *it##_tmp = container_of(it->member.prev, type, member); \
            &it->member != (head); it = it##_tmp, it##_tmp = container_of(it->member.prev, type, member))

#endif
