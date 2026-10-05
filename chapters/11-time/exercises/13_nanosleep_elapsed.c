/*
 * Exercise 11.13 - Sleep with nanosecond resolution
 *
 * Purpose:
 *   Request a short relative sleep and measure elapsed monotonic time.
 *
 * Linux behavior:
 *   nanosleep() suspends for at least the requested interval unless a signal
 *   interrupts it. Actual wake-up can be later because the scheduler must make
 *   the process runnable and dispatch it.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long long to_nanoseconds(const struct timespec *value)
{
    return (long long)value->tv_sec * 1000000000LL +
           (long long)value->tv_nsec;
}

static int sleep_completely(struct timespec request)
{
    while (nanosleep(&request, &request) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }
    return 0;
}

int main(void)
{
    const long long requested_ns = 5000000LL;
    const struct timespec request = {.tv_sec = 0, .tv_nsec = requested_ns};
    struct timespec before;
    struct timespec after;
    long long elapsed;

    if (clock_gettime(CLOCK_MONOTONIC, &before) == -1 ||
        sleep_completely(request) == -1 ||
        clock_gettime(CLOCK_MONOTONIC, &after) == -1) {
        perror("nanosleep measurement");
        return EXIT_FAILURE;
    }

    elapsed = to_nanoseconds(&after) - to_nanoseconds(&before);
    printf("requested_ns=%lld elapsed_at_least_requested=%s\n",
           requested_ns,
           elapsed >= requested_ns ? "yes" : "no");
    printf("elapsed_clock=monotonic late_wakeup_allowed=yes interruption_retry_supported=yes\n");

    return elapsed >= requested_ns ? EXIT_SUCCESS : EXIT_FAILURE;
}
