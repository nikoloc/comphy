#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include <stdint.h>
#include <time.h>

static inline uint64_t
time_from_timespec_ms(struct timespec *ts) {
    return ts->tv_sec * 1000 + ts->tv_nsec / 1000000;
}

static inline uint64_t
time_from_timespec_ns(struct timespec *ts) {
    return (uint64_t)ts->tv_sec * 1000000000 + ts->tv_nsec;
}

static inline struct timespec
time_now_timespec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts;
}

static inline uint64_t
time_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return time_from_timespec_ms(&ts);
}

static inline uint64_t
time_now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return time_from_timespec_ns(&ts);
}

static inline uint64_t
time_delta_ns(struct timespec *a, struct timespec *b) {
    return (double)(time_from_timespec_ns(a) - time_from_timespec_ns(b));
}

static inline uint64_t
time_delta_ms(struct timespec *a, struct timespec *b) {
    return time_delta_ns(a, b) / 1000000.0F;
}

#endif
