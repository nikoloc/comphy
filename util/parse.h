#ifndef PARSE_H
#define PARSE_H

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

static inline bool
parse_int(char *str, int *dest) {
    if(str == NULL) {
        return false;
    }

    char *end;
    errno = 0;
    long result = strtol(str, &end, 0);

    if(end == str || *end != '\0' || result < INT_MIN || result > INT_MAX || errno == ERANGE) {
        return false;
    }

    *dest = (int)result;
    return true;
}

static inline bool
parse_float(char *str, float *dest) {
    if(str == NULL) {
        return false;
    }

    char *end;
    errno = 0;
    float result = strtof(str, &end);

    if(end == str || *end != '\0' || errno == ERANGE) {
        return false;
    }

    *dest = result;
    return true;
}

#endif
