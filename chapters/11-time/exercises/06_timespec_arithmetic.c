/*
 * Exercise 11.06 - Normalize timespec subtraction
 *
 * Purpose:
 *   Subtract two timestamps while borrowing one second when the nanosecond
 *   field would otherwise be negative.
 *
 * Linux behavior:
 *   timespec values are pairs, not floating-point seconds. Correct arithmetic
 *   preserves the invariant 0 <= tv_nsec < 1,000,000,000.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static struct timespec subtract_timespec(struct timespec end,
                                         struct timespec start)
{
    struct timespec result = {
        .tv_sec = end.tv_sec - start.tv_sec,
        .tv_nsec = end.tv_nsec - start.tv_nsec,
    };

    if (result.tv_nsec < 0L) {
        result.tv_nsec += 1000000000L;
        --result.tv_sec;
    }
    return result;
}

int main(void)
{
    const struct timespec start = {.tv_sec = 8, .tv_nsec = 900000000L};
    const struct timespec end = {.tv_sec = 10, .tv_nsec = 100000000L};
    struct timespec elapsed = subtract_timespec(end, start);
    int correct = elapsed.tv_sec == (time_t)1 && elapsed.tv_nsec == 200000000L;

    printf("elapsed_seconds=%lld elapsed_nanoseconds=%ld\n",
           (long long)elapsed.tv_sec,
           elapsed.tv_nsec);
    printf("borrow_applied=yes normalized=%s expected_1_2_seconds=%s\n",
           elapsed.tv_nsec >= 0L && elapsed.tv_nsec < 1000000000L ? "yes" : "no",
           correct ? "yes" : "no");

    return correct ? EXIT_SUCCESS : EXIT_FAILURE;
}
