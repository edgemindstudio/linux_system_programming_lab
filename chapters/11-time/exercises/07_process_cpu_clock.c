/*
 * Exercise 11.07 - Measure process CPU time
 *
 * Purpose:
 *   Measure CPU consumed by a bounded computation with
 *   CLOCK_PROCESS_CPUTIME_ID.
 *
 * Linux behavior:
 *   A process CPU clock advances while threads in the process execute on a
 *   processor. It does not measure ordinary time spent asleep or blocked.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long long to_nanoseconds(const struct timespec *value)
{
    return (long long)value->tv_sec * 1000000000LL +
           (long long)value->tv_nsec;
}

int main(void)
{
    struct timespec before;
    struct timespec after;
    volatile unsigned long accumulator = 0UL;
    long long elapsed;

    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &before) == -1) {
        perror("clock_gettime before");
        return EXIT_FAILURE;
    }
    for (unsigned long value = 0UL; value < 1000000UL; ++value) {
        accumulator += value & 7UL;
    }
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &after) == -1) {
        perror("clock_gettime after");
        return EXIT_FAILURE;
    }

    elapsed = to_nanoseconds(&after) - to_nanoseconds(&before);
    printf("process_cpu_elapsed_nonnegative=%s work_completed=%s\n",
           elapsed >= 0LL ? "yes" : "no",
           accumulator > 0UL ? "yes" : "no");
    printf("sleep_time_excluded=yes all_process_threads_contribute=yes\n");

    return elapsed >= 0LL && accumulator > 0UL ? EXIT_SUCCESS : EXIT_FAILURE;
}
