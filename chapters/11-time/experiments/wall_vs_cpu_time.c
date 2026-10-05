/*
 * Experiment - Compare wall elapsed time with consumed CPU time
 *
 * Prediction:
 *   Sleeping advances CLOCK_MONOTONIC while adding very little to the process
 *   CPU clock.
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

int main(void)
{
    const struct timespec request = {.tv_sec = 0, .tv_nsec = 30000000L};
    struct timespec wall_before;
    struct timespec wall_after;
    struct timespec cpu_before;
    struct timespec cpu_after;
    struct timespec remaining = request;
    long long wall_elapsed;
    long long cpu_elapsed;

    if (clock_gettime(CLOCK_MONOTONIC, &wall_before) == -1 ||
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_before) == -1) {
        perror("clock_gettime before");
        return EXIT_FAILURE;
    }
    while (nanosleep(&remaining, &remaining) == -1) {
        if (errno != EINTR) {
            perror("nanosleep");
            return EXIT_FAILURE;
        }
    }
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_after) == -1 ||
        clock_gettime(CLOCK_MONOTONIC, &wall_after) == -1) {
        perror("clock_gettime after");
        return EXIT_FAILURE;
    }

    wall_elapsed = to_nanoseconds(&wall_after) - to_nanoseconds(&wall_before);
    cpu_elapsed = to_nanoseconds(&cpu_after) - to_nanoseconds(&cpu_before);
    printf("wall_elapsed_positive=%s cpu_elapsed_nonnegative=%s\n",
           wall_elapsed > 0LL ? "yes" : "no",
           cpu_elapsed >= 0LL ? "yes" : "no");
    printf("wall_exceeds_cpu_during_sleep=%s blocked_time_is_not_cpu_time=yes\n",
           wall_elapsed > cpu_elapsed ? "yes" : "no");

    return wall_elapsed > 0LL && cpu_elapsed >= 0LL && wall_elapsed > cpu_elapsed
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
