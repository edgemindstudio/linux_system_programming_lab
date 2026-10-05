/*
 * Experiment - Compare relative and absolute periodic scheduling
 *
 * Prediction:
 *   Repeating "work, then sleep one period" accumulates work time. Reusing a
 *   fixed sequence of absolute deadlines keeps each wake-up anchored to the
 *   original schedule.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum { PERIOD_COUNT = 5 };

static long long to_nanoseconds(const struct timespec *value)
{
    return (long long)value->tv_sec * 1000000000LL +
           (long long)value->tv_nsec;
}

static void add_nanoseconds(struct timespec *value, long nanoseconds)
{
    value->tv_nsec += nanoseconds;
    while (value->tv_nsec >= 1000000000L) {
        ++value->tv_sec;
        value->tv_nsec -= 1000000000L;
    }
}

static int relative_schedule(long long *elapsed)
{
    const struct timespec period = {.tv_sec = 0, .tv_nsec = 5000000L};
    const struct timespec work = {.tv_sec = 0, .tv_nsec = 2000000L};
    struct timespec before;
    struct timespec after;

    if (clock_gettime(CLOCK_MONOTONIC, &before) == -1) {
        return -1;
    }
    for (int iteration = 0; iteration < PERIOD_COUNT; ++iteration) {
        (void)nanosleep(&period, NULL);
        (void)nanosleep(&work, NULL);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &after) == -1) {
        return -1;
    }
    *elapsed = to_nanoseconds(&after) - to_nanoseconds(&before);
    return 0;
}

static int absolute_schedule(long long *elapsed)
{
    const struct timespec work = {.tv_sec = 0, .tv_nsec = 2000000L};
    struct timespec before;
    struct timespec deadline;
    struct timespec after;

    if (clock_gettime(CLOCK_MONOTONIC, &before) == -1) {
        return -1;
    }
    deadline = before;
    for (int iteration = 0; iteration < PERIOD_COUNT; ++iteration) {
        int result;

        add_nanoseconds(&deadline, 5000000L);
        do {
            result = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                                     &deadline, NULL);
        } while (result == EINTR);
        if (result != 0) {
            return -1;
        }
        (void)nanosleep(&work, NULL);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &after) == -1) {
        return -1;
    }
    *elapsed = to_nanoseconds(&after) - to_nanoseconds(&before);
    return 0;
}

int main(void)
{
    long long relative_elapsed;
    long long absolute_elapsed;

    if (relative_schedule(&relative_elapsed) == -1 ||
        absolute_schedule(&absolute_elapsed) == -1) {
        perror("schedule measurement");
        return EXIT_FAILURE;
    }

    printf("periods=%d relative_elapsed_positive=%s absolute_elapsed_positive=%s\n",
           PERIOD_COUNT,
           relative_elapsed > 0LL ? "yes" : "no",
           absolute_elapsed > 0LL ? "yes" : "no");
    printf("absolute_faster_in_this_run=%s absolute_schedule_uses_fixed_deadlines=yes\n",
           absolute_elapsed < relative_elapsed ? "yes" : "no");

    return relative_elapsed > 0LL && absolute_elapsed > 0LL
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
