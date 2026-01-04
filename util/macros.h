#ifndef MACROS_H
#define MACROS_H

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define clamp(v, a, b) (max(min((v), (b)), (a)))

#define fequal(f, v) (fabsf((f) - (v)) < 1e-6)

#define unused(x) ((void)(x))
#define todo(x) (assert(0 && "todo"))

#define container_of(ptr, type, member) (type *)((char *)(ptr) - offsetof(type, member))

#define starts_with(s, t) (strncmp(s, t, strlen(t)))

#endif
