/*
 * Experiment - Sample realtime and monotonic clocks repeatedly
 *
 * Prediction:
 *   CLOCK_MONOTONIC never moves backward. CLOCK_REALTIME normally advances
 *   too, but administrators and synchronization services may correct it.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum { SAMPLE_COUNT = 5 };

static int earlier(const struct timespec *left, const struct timespec *right)
{
    return left->tv_sec < right->tv_sec ||
           (left->tv_sec == right->tv_sec && left->tv_nsec < right->tv_nsec);
}

int main(void)
{
    const struct timespec delay = {.tv_sec = 0, .tv_nsec = 2000000L};
    struct timespec previous_realtime;
    struct timespec previous_monotonic;
    int monotonic_non_decreasing = 1;
    int realtime_non_decreasing = 1;

    if (clock_gettime(CLOCK_REALTIME, &previous_realtime) == -1 ||
        clock_gettime(CLOCK_MONOTONIC, &previous_monotonic) == -1) {
        perror("clock_gettime initial");
        return EXIT_FAILURE;
    }

    for (int sample = 1; sample < SAMPLE_COUNT; ++sample) {
        struct timespec realtime;
        struct timespec monotonic;

        (void)nanosleep(&delay, NULL);
        if (clock_gettime(CLOCK_REALTIME, &realtime) == -1 ||
            clock_gettime(CLOCK_MONOTONIC, &monotonic) == -1) {
            perror("clock_gettime sample");
            return EXIT_FAILURE;
        }
        if (earlier(&monotonic, &previous_monotonic)) {
            monotonic_non_decreasing = 0;
        }
        if (earlier(&realtime, &previous_realtime)) {
            realtime_non_decreasing = 0;
        }
        previous_monotonic = monotonic;
        previous_realtime = realtime;
    }

    printf("samples=%d monotonic_non_decreasing=%s\n",
           SAMPLE_COUNT,
           monotonic_non_decreasing ? "yes" : "no");
    printf("realtime_non_decreasing_in_this_run=%s realtime_can_be_corrected=yes\n",
           realtime_non_decreasing ? "yes" : "no");

    return monotonic_non_decreasing ? EXIT_SUCCESS : EXIT_FAILURE;
}
