#ifndef LIST_HELPERS_H
#define LIST_HELPERS_H

#include <wayland-server-protocol.h>

static inline struct wl_list *
wl_list_get_next_or_prev(struct wl_list *list, struct wl_list *elem) {
    struct wl_list *next = elem->next;
    if(next != list) {
        return next;
    }

    struct wl_list *prev = elem->prev;
    if(prev != list) {
        return prev;
    }

    return NULL;
}

static inline struct wl_list *
wl_list_get_prev_or_next(struct wl_list *list, struct wl_list *elem) {
    struct wl_list *prev = elem->prev;
    if(prev != list) {
        return prev;
    }

    struct wl_list *next = elem->next;
    if(next != list) {
        return next;
    }

    return NULL;
}

static inline struct wl_list *
wl_list_first(struct wl_list *list) {
    if(list->next == list) {
        return NULL;
    }

    return list->next;
}

static inline struct wl_list *
wl_list_last(struct wl_list *list) {
    if(list->prev == list) {
        return NULL;
    }

    return list->prev;
}

static inline bool
wl_list_is_first(struct wl_list *list, struct wl_list *elem) {
    return elem->prev == list;
}

static inline bool
wl_list_is_last(struct wl_list *list, struct wl_list *elem) {
    return elem->next == list;
}

#endif
