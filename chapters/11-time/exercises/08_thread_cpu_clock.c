/*
 * Exercise 11.08 - Measure current-thread CPU time
 *
 * Purpose:
 *   Sample CLOCK_THREAD_CPUTIME_ID around work performed by the calling thread.
 *
 * Linux behavior:
 *   A thread CPU clock accounts only for the current kernel task. It is useful
 *   when process-wide CPU time would combine unrelated worker activity.
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
    volatile unsigned long accumulator = 1UL;
    long long elapsed;

    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &before) == -1) {
        perror("clock_gettime before");
        return EXIT_FAILURE;
    }
    for (unsigned long value = 1UL; value < 750000UL; ++value) {
        accumulator ^= value;
    }
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &after) == -1) {
        perror("clock_gettime after");
        return EXIT_FAILURE;
    }

    elapsed = to_nanoseconds(&after) - to_nanoseconds(&before);
    printf("thread_cpu_elapsed_nonnegative=%s computation_observed=%s\n",
           elapsed >= 0LL ? "yes" : "no",
           accumulator != 0UL ? "yes" : "no");
    printf("clock_scope=current-thread process_clock_is_broader=yes\n");

    return elapsed >= 0LL ? EXIT_SUCCESS : EXIT_FAILURE;
}
