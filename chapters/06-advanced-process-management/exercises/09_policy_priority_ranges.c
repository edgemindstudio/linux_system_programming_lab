/*
 * Exercise 06.09 - Discover scheduling priority ranges
 *
 * Purpose:
 *   Ask Linux for policy-specific priority ranges rather than hardcoding
 *   assumptions into a program.
 *
 * Linux behavior:
 *   SCHED_OTHER normally has only priority 0. Linux real-time policies expose
 *   a positive range, commonly 1 through 99. Portable code still queries the
 *   range because POSIX does not require those exact endpoints.
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

static int range_for(int policy, int *minimum, int *maximum)
{
    *minimum = sched_get_priority_min(policy);
    if (*minimum == -1) {
        return -1;
    }
    *maximum = sched_get_priority_max(policy);
    return *maximum == -1 ? -1 : 0;
}

int main(void)
{
    int other_min;
    int other_max;
    int fifo_min;
    int fifo_max;
    int rr_min;
    int rr_max;

    if (range_for(SCHED_OTHER, &other_min, &other_max) == -1 ||
        range_for(SCHED_FIFO, &fifo_min, &fifo_max) == -1 ||
        range_for(SCHED_RR, &rr_min, &rr_max) == -1) {
        perror("sched_get_priority range");
        return EXIT_FAILURE;
    }

    printf("other=%d..%d fifo=%d..%d rr=%d..%d\n",
           other_min, other_max, fifo_min, fifo_max, rr_min, rr_max);
    printf("normal_priority_zero=%s realtime_ranges_valid=%s\n",
           other_min == 0 && other_max == 0 ? "yes" : "no",
           fifo_min > 0 && fifo_max >= fifo_min &&
                   rr_min > 0 && rr_max >= rr_min
               ? "yes" : "no");

    return EXIT_SUCCESS;
}
