/*
 * Exercise 11.05 - Query clock resolution
 *
 * Purpose:
 *   Ask the kernel for the nominal resolution of the realtime and monotonic
 *   clocks instead of inferring precision from a timespec field width.
 *
 * Linux behavior:
 *   clock_getres() reports the granularity of a clock source. Resolution,
 *   precision, accuracy, and scheduler wake-up latency are different concepts.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int positive_resolution(const struct timespec *resolution)
{
    return resolution->tv_sec > (time_t)0 || resolution->tv_nsec > 0L;
}

int main(void)
{
    struct timespec realtime;
    struct timespec monotonic;

    if (clock_getres(CLOCK_REALTIME, &realtime) == -1 ||
        clock_getres(CLOCK_MONOTONIC, &monotonic) == -1) {
        perror("clock_getres");
        return EXIT_FAILURE;
    }

    printf("realtime_resolution_positive=%s monotonic_resolution_positive=%s\n",
           positive_resolution(&realtime) ? "yes" : "no",
           positive_resolution(&monotonic) ? "yes" : "no");
    printf("field_width_is_not_accuracy=yes wakeup_latency_is_separate=yes\n");

    return positive_resolution(&realtime) && positive_resolution(&monotonic)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
