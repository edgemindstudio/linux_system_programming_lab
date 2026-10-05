/*
 * Exercise 11.04 - Distinguish realtime from monotonic time
 *
 * Purpose:
 *   Sample CLOCK_REALTIME and CLOCK_MONOTONIC and classify their intended
 *   uses rather than comparing their unrelated numeric origins.
 *
 * Linux behavior:
 *   Realtime represents civil wall time and may jump when corrected.
 *   Monotonic time has an unspecified origin and does not move backward,
 *   making it the normal choice for elapsed-time measurement.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int valid_timespec(const struct timespec *value)
{
    return value->tv_sec >= (time_t)0 && value->tv_nsec >= 0L &&
           value->tv_nsec < 1000000000L;
}

int main(void)
{
    struct timespec realtime;
    struct timespec monotonic;

    if (clock_gettime(CLOCK_REALTIME, &realtime) == -1) {
        perror("clock_gettime realtime");
        return EXIT_FAILURE;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &monotonic) == -1) {
        perror("clock_gettime monotonic");
        return EXIT_FAILURE;
    }

    printf("realtime_valid=%s monotonic_valid=%s\n",
           valid_timespec(&realtime) ? "yes" : "no",
           valid_timespec(&monotonic) ? "yes" : "no");
    printf("timestamps_use=realtime elapsed_intervals_use=monotonic origins_compared=no\n");

    return valid_timespec(&realtime) && valid_timespec(&monotonic)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
