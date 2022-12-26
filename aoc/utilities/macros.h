#pragma once

#include "libc/macros.internal.h"
#include "libc/calls/struct/timespec.h"
#include "libc/sysv/consts/clock.h"

#define LENGTH(x) (sizeof(STRINGIFY(x)) - 1)

#define runTask(stringName, funcName)                       \
    puts("Running " stringName);                            \
    funcName();                                             \
    putchar('\n');

// TODO Support cleanup between runs rather than having to do it within them
// Based on https://levelup.gitconnected.com/8-ways-to-measure-execution-time-in-c-c-48634458d0f9
#if TESTING
    
#define startTesting()                                      \
    struct timespec begin, end;                             \
    clock_gettime(CLOCK_REALTIME, &begin);                  \
    uint_fast32_t iterations = 1000*100;                    \
    for (int i=0; i<iterations; i++) {

#define endTesting()                                        \
    }                                                       \
    clock_gettime(CLOCK_REALTIME, &end);                    \
    int64_t seconds = end.tv_sec - begin.tv_sec;            \
    int64_t nanoseconds = end.tv_nsec - begin.tv_nsec;      \
    double elapsed = seconds + nanoseconds*1e-9;            \
    double singleElapsed = elapsed / (double)iterations;    \
    printf("Time measured: %.8f seconds\n", singleElapsed);

#else

#define startTesting()

#define endTesting()

#endif