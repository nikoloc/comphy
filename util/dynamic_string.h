#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef STRING_DEFAULT_CAP
#define STRING_DEFAULT_CAP 16
#endif

typedef struct string {
    int len, cap;
    char *data;
} string_t;

static inline void
string_reserve(string_t *s, int cap) {
    if(cap < STRING_DEFAULT_CAP) {
        cap = STRING_DEFAULT_CAP;
    }

    if(cap <= s->cap) {
        return;
    }

    s->cap = cap;
    s->data = realloc(s->data, cap * sizeof(char));
}

static inline void
string_init(string_t *s, char *v) {
    int len;
    if(!v || (len = strlen(v)) == 0) {
        memset(s, 0, sizeof(*s));
        return;
    }

    s->len = len;
    string_reserve(s, len);

    memcpy(s->data, v, s->len);
}

static inline void
string_deinit(string_t *s) {
    if(s->cap > 0) {
        free(s->data);
    }
}

static inline void
string_assign(string_t *s, char *v) {
    s->len = strlen(v);
    string_reserve(s, s->len);
    memcpy(s->data, v, s->len);
}

static inline void
string_clone(string_t *dest, string_t *src) {
    if(src->len == 0) {
        string_init(dest, NULL);
        return;
    }

    dest->len = src->len;
    string_reserve(dest, dest->len);
    memcpy(dest->data, src->data, src->len);
}

static inline bool
string_equal(string_t *a, string_t *b) {
    if(a->len != b->len) {
        return false;
    }

    return memcmp(a->data, b->data, a->len) == 0;
}

static inline int
string_index_of_from(string_t *s, char c, int start) {
    for(int i = start; i < s->len; i++) {
        if(s->data[i] == c) {
            return i;
        }
    }

    return -1;
}

static inline int
string_index_of(string_t *s, char c) {
    return string_index_of_from(s, c, 0);
}

static inline int
string_index_of_from_reverse(string_t *s, char c, int start) {
    for(int i = start; i >= 0; i--) {
        if(s->data[i] == c) {
            return i;
        }
    }

    return -1;
}

static inline int
string_index_of_reverse(string_t *s, char c) {
    return string_index_of_from_reverse(s, c, s->len - 1);
}

static inline int
string_index_of_any_from(string_t *s, char *c, char *which, int start) {
    for(int i = start; i < s->len; i++) {
        for(char *t = c; *t != 0; t++) {
            if(s->data[i] == *t) {
                if(which) {
                    *which = *t;
                }

                return i;
            }
        }
    }

    return -1;
}

static inline int
string_index_of_any(string_t *s, char *c, char *which) {
    return string_index_of_any_from(s, c, which, 0);
}

static inline void
string_substring(string_t *dest, string_t *src, int start, int end) {
    int len = end - start;

    if(len <= 0) {
        string_init(dest, NULL);
        return;
    }

    dest->len = len;
    string_reserve(dest, len);
    memcpy(dest->data, src->data + start, len);
}

static inline void
string_append(string_t *s, char c) {
    if(s->len == s->cap) {
        string_reserve(s, 2 * s->cap);
    }

    s->data[s->len] = c;
    s->len++;
}

static inline void
string_append_n(string_t *s, int n, char *c) {
    int new_len = s->len + n;
    if(new_len > s->cap) {
        string_reserve(s, new_len);
    }

    memcpy(s->data + s->len, c, n);
    s->len = new_len;
}

static inline void
string_append_string(string_t *s, string_t *a) {
    int new_len = s->len + a->len;
    if(new_len > s->cap) {
        string_reserve(s, new_len);
    }

    memcpy(s->data + s->len, a->data, a->len);
    s->len = new_len;
}

static inline void
string_append_c_string(string_t *s, char *a) {
    for(char *p = a; *p != 0; p++) {
        string_append(s, *p);
    }
}

static inline char *
string_c_string_view(string_t *s) {
    if(s->len == s->cap) {
        string_reserve(s, s->cap + 1);
    }

    s->data[s->len] = 0;
    return s->data;
}

static inline bool
string_equal_c_string(string_t *a, char *b) {
    return strcmp(string_c_string_view(a), b) == 0;
}

// only enable this functionality if `array.h` is included
#ifdef ARRAY_H
define_array(string_t, string_array);

static inline void
string_split(string_t *s, char c, bool ignore_multiple, string_array_t *dest) {
    dest->len = 0;

    int start = 0, end;
    while((end = string_index_of_from(s, c, start)) != -1) {
        if(ignore_multiple && start == end) {
            start++;
            continue;
        }

        string_t sub = {0};
        string_substring(&sub, s, start, end);
        string_array_push(dest, sub);
        start = end + 1;
    }

    // add the last one
    if(start != s->len) {
        string_t sub = {0};
        string_substring(&sub, s, start, s->len);
        string_array_push(dest, sub);
    }
}

static inline void
string_shell_split(string_t *s, string_array_t *dest) {
    dest->len = 0;
    int i = 0;

    while(i < s->len) {
        while(i < s->len && s->data[i] == ' ') {
            i++;
        }

        if(i >= s->len) {
            break;
        }

        string_t token = {0};
        bool in_quotes = false;

        while(i < s->len) {
            char c = s->data[i];

            if(c == '"') {
                in_quotes = !in_quotes;
                i++;
                continue;
            }

            if(c == '\\' && i + 1 < s->len) {
                // handle escape sequences
                char next = s->data[i + 1];
                if(next == '"' || next == '\\') {
                    string_append(&token, next);
                    i += 2;
                    continue;
                }
            }

            if(!in_quotes && c == ' ') {
                // delimiter outside quotes ends token
                i++;
                break;
            }

            string_append(&token, c);
            i++;
        }

        string_array_push(dest, token);
    }
}
#endif

#endif
