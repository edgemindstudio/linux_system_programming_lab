/*
 * Exercise 11.15 - Sleep until an absolute monotonic deadline
 *
 * Purpose:
 *   Build a future CLOCK_MONOTONIC timestamp and wait for it with
 *   clock_nanosleep(TIMER_ABSTIME).
 *
 * Linux behavior:
 *   Absolute deadlines avoid adding work duration and restart overhead to
 *   every period. clock_nanosleep() returns an error number directly.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void add_nanoseconds(struct timespec *value, long nanoseconds)
{
    value->tv_nsec += nanoseconds;
    if (value->tv_nsec >= 1000000000L) {
        ++value->tv_sec;
        value->tv_nsec -= 1000000000L;
    }
}

static int at_or_after(const struct timespec *value,
                       const struct timespec *deadline)
{
    return value->tv_sec > deadline->tv_sec ||
           (value->tv_sec == deadline->tv_sec &&
            value->tv_nsec >= deadline->tv_nsec);
}

int main(void)
{
    struct timespec deadline;
    struct timespec after;
    int result;

    if (clock_gettime(CLOCK_MONOTONIC, &deadline) == -1) {
        perror("clock_gettime deadline");
        return EXIT_FAILURE;
    }
    add_nanoseconds(&deadline, 10000000L);
    do {
        result = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                                 &deadline, NULL);
    } while (result == EINTR);
    if (result != 0) {
        fprintf(stderr, "clock_nanosleep: %s\n", strerror(result));
        return EXIT_FAILURE;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &after) == -1) {
        perror("clock_gettime after");
        return EXIT_FAILURE;
    }

    printf("absolute_sleep_completed=yes reached_deadline=%s\n",
           at_or_after(&after, &deadline) ? "yes" : "no");
    printf("clock=monotonic drift_from_repeated_relative_sleeps_avoided=yes\n");

    return at_or_after(&after, &deadline) ? EXIT_SUCCESS : EXIT_FAILURE;
}
