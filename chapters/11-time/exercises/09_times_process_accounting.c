/*
 * Exercise 11.09 - Read POSIX process accounting ticks
 *
 * Purpose:
 *   Use times() and sysconf(_SC_CLK_TCK) to inspect elapsed, user, and system
 *   accounting values without assuming a fixed tick frequency.
 *
 * Linux behavior:
 *   times() exports clock_t values in units described by _SC_CLK_TCK. Short
 *   work may consume less than one accounting tick, so zero deltas are valid.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>

int main(void)
{
    struct tms before_usage;
    struct tms after_usage;
    clock_t before;
    clock_t after;
    long ticks_per_second = sysconf(_SC_CLK_TCK);
    volatile unsigned long work = 0UL;

    if (ticks_per_second <= 0L) {
        perror("sysconf _SC_CLK_TCK");
        return EXIT_FAILURE;
    }
    before = times(&before_usage);
    if (before == (clock_t)-1) {
        perror("times before");
        return EXIT_FAILURE;
    }
    for (unsigned long value = 0UL; value < 500000UL; ++value) {
        work += value & 3UL;
    }
    after = times(&after_usage);
    if (after == (clock_t)-1) {
        perror("times after");
        return EXIT_FAILURE;
    }

    printf("ticks_per_second_positive=yes elapsed_ticks_nonnegative=%s\n",
           after >= before ? "yes" : "no");
    printf("user_ticks_nonnegative=%s system_ticks_nonnegative=%s work_completed=%s\n",
           after_usage.tms_utime >= before_usage.tms_utime ? "yes" : "no",
           after_usage.tms_stime >= before_usage.tms_stime ? "yes" : "no",
           work > 0UL ? "yes" : "no");

    return after >= before && work > 0UL ? EXIT_SUCCESS : EXIT_FAILURE;
}
