/*
 * Exercise 06.11 - Query the round-robin interval
 *
 * Purpose:
 *   Read the scheduling quantum reported for the current process.
 *
 * Linux behavior:
 *   POSIX requires sched_rr_get_interval() for SCHED_RR processes. Linux also
 *   answers for normal processes. The result describes scheduler accounting;
 *   it is not a promise that a process will execute uninterrupted for exactly
 *   this duration.
 */

#define _POSIX_C_SOURCE 200809L

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    struct timespec interval;
    long long total_nanoseconds;

    if (sched_rr_get_interval(0, &interval) == -1) {
        perror("sched_rr_get_interval");
        return EXIT_FAILURE;
    }

    total_nanoseconds =
        (long long)interval.tv_sec * 1000000000LL + interval.tv_nsec;

    printf("interval_sec=%lld interval_nsec=%ld total_ns=%lld\n",
           (long long)interval.tv_sec, interval.tv_nsec, total_nanoseconds);
    printf("interval_query=yes representation_valid=%s\n",
           interval.tv_sec >= 0 && interval.tv_nsec >= 0 &&
                   interval.tv_nsec < 1000000000L
               ? "yes" : "no");

    return EXIT_SUCCESS;
}
