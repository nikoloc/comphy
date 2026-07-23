#ifndef DSTRING_H
#define DSTRING_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct string {
    size_t len, cap;
    char *data;
} string_t;

void
string_reserve(string_t *s, size_t cap);

void
string_init(string_t *s, char *v);

void
string_deinit(string_t *s);

void
string_assign(string_t *s, char *v);

void
string_clone(string_t *dest, string_t *src);

bool
string_equal(string_t *a, string_t *b);

ssize_t
string_index_of_from(string_t *s, char c, size_t start);

ssize_t
string_index_of(string_t *s, char c);

ssize_t
string_index_of_from_reverse(string_t *s, char c, ssize_t start);

ssize_t
string_index_of_reverse(string_t *s, char c);

ssize_t
string_index_of_any_from(string_t *s, char *c, char *which, ssize_t start);

ssize_t
string_index_of_any(string_t *s, char *c, char *which);

void
string_substring(string_t *dest, string_t *src, size_t start, size_t end);

void
string_append(string_t *s, char c);

void
string_append_many(string_t *s, size_t n, char *c);

void
string_append_string(string_t *s, string_t *a);

void
string_append_c_string(string_t *s, char *a);

char *
string_c_string_view(string_t *s);

bool
string_equal_c_string(string_t *a, char *b);

#endif

#ifdef DSTRING_IMPLEMENTATION

#ifndef DSTRING_DEFAULT_CAP
#define DSTRING_DEFAULT_CAP 16
#endif

void
string_reserve(string_t *s, size_t cap) {
    if(cap <= s->cap) {
        return;
    }

    s->cap = cap;
    s->data = realloc(s->data, cap * sizeof(char));
}

void
string_init(string_t *s, char *v) {
    size_t len;
    if(!v || (len = strlen(v)) == 0) {
        memset(s, 0, sizeof(*s));
        return;
    }

    s->len = len;
    string_reserve(s, len);

    memcpy(s->data, v, s->len);
}

void
string_deinit(string_t *s) {
    if(s->cap > 0) {
        free(s->data);
    }
}

void
string_assign(string_t *s, char *v) {
    s->len = strlen(v);
    string_reserve(s, s->len);
    memcpy(s->data, v, s->len);
}
void
string_clone(string_t *dest, string_t *src) {
    if(src->len == 0) {
        string_init(dest, NULL);
        return;
    }

    dest->len = src->len;
    string_reserve(dest, dest->len);
    memcpy(dest->data, src->data, src->len);
}

bool
string_equal(string_t *a, string_t *b) {
    if(a->len != b->len) {
        return false;
    }

    return memcmp(a->data, b->data, a->len) == 0;
}

ssize_t
string_index_of_from(string_t *s, char c, size_t start) {
    for(size_t i = start; i < s->len; i++) {
        if(s->data[i] == c) {
            return i;
        }
    }

    return -1;
}

ssize_t
string_index_of(string_t *s, char c) {
    return string_index_of_from(s, c, 0);
}

ssize_t
string_index_of_from_reverse(string_t *s, char c, ssize_t start) {
    for(ssize_t i = start; i >= 0; i--) {
        if(s->data[i] == c) {
            return i;
        }
    }

    return -1;
}

ssize_t
string_index_of_reverse(string_t *s, char c) {
    return string_index_of_from_reverse(s, c, s->len - 1);
}

ssize_t
string_index_of_any_from(string_t *s, char *c, char *which, ssize_t start) {
    for(size_t i = start; i < s->len; i++) {
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

ssize_t
string_index_of_any(string_t *s, char *c, char *which) {
    return string_index_of_any_from(s, c, which, 0);
}

void
string_substring(string_t *dest, string_t *src, size_t start, size_t end) {
    size_t len = end - start;

    if(len <= 0) {
        string_init(dest, NULL);
        return;
    }

    dest->len = len;
    string_reserve(dest, len);
    memcpy(dest->data, src->data + start, len);
}

void
string_append(string_t *s, char c) {
    if(s->cap == 0) {
        string_reserve(s, DSTRING_DEFAULT_CAP);
    } else if(s->len == s->cap) {
        string_reserve(s, 2 * s->cap);
    }

    s->data[s->len] = c;
    s->len++;
}

void
string_append_many(string_t *s, size_t n, char *c) {
    size_t new_len = s->len + n;
    if(new_len > s->cap) {
        string_reserve(s, new_len);
    }

    memcpy(s->data + s->len, c, n);
    s->len = new_len;
}

void
string_append_string(string_t *s, string_t *a) {
    size_t new_len = s->len + a->len;
    if(new_len > s->cap) {
        string_reserve(s, new_len);
    }

    memcpy(s->data + s->len, a->data, a->len);
    s->len = new_len;
}

void
string_append_c_string(string_t *s, char *a) {
    for(char *p = a; *p != 0; p++) {
        string_append(s, *p);
    }
}

char *
string_c_string_view(string_t *s) {
    if(s->cap == 0) {
        string_reserve(s, DSTRING_DEFAULT_CAP);
    } else if(s->len == s->cap) {
        string_reserve(s, s->cap + 1);
    }

    s->data[s->len] = 0;
    return s->data;
}

bool
string_equal_c_string(string_t *a, char *b) {
    return strcmp(string_c_string_view(a), b) == 0;
}

// only enable this functionality if `array.h` is included
#ifdef ARRAY_H
DEFINE_ARRAY(string_t, string_array);

void
string_split(string_t *s, char c, bool ignore_multiple, string_array_t *dest) {
    dest->len = 0;

    size_t start = 0, end;
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

void
string_shell_split(string_t *s, string_array_t *dest) {
    dest->len = 0;
    size_t i = 0;

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

void
string_split_destroy_all(string_array_t *result) {
    for(size_t i = 0; i < result->len; i++) {
        string_deinit(&result->data[i]);
    }
}
#endif

#endif
