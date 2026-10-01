/*
 * Exercise 06.02 - Voluntarily yield the processor
 *
 * Purpose:
 *   Call sched_yield() and measure the elapsed monotonic time around it.
 *
 * Linux behavior:
 *   sched_yield() moves the caller behind other runnable tasks at the same
 *   static priority. It does not sleep for a requested duration and it does
 *   not guarantee that another process will run. On an otherwise idle CPU,
 *   the caller may resume almost immediately.
 */

#define _POSIX_C_SOURCE 200809L

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long long nanoseconds_between(const struct timespec *start,
                                     const struct timespec *end)
{
    long long seconds = (long long)end->tv_sec - (long long)start->tv_sec;
    long long nanoseconds = (long long)end->tv_nsec - (long long)start->tv_nsec;

    return seconds * 1000000000LL + nanoseconds;
}

int main(void)
{
    struct timespec before;
    struct timespec after;
    long long elapsed;

    if (clock_gettime(CLOCK_MONOTONIC, &before) == -1) {
        perror("clock_gettime before");
        return EXIT_FAILURE;
    }

    if (sched_yield() == -1) {
        perror("sched_yield");
        return EXIT_FAILURE;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &after) == -1) {
        perror("clock_gettime after");
        return EXIT_FAILURE;
    }

    elapsed = nanoseconds_between(&before, &after);
    printf("sched_yield_result=0 elapsed_ns=%lld\n", elapsed);
    printf("elapsed_nonnegative=%s yield_is_not_sleep=yes\n",
           elapsed >= 0 ? "yes" : "no");

    return EXIT_SUCCESS;
}
