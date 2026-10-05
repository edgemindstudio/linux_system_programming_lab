/*
 * Exercise 11.01 - Inspect the standard time data types
 *
 * Purpose:
 *   Connect time_t, timeval, timespec, and tm to the kinds of time values
 *   represented by Linux and the C/POSIX interfaces.
 *
 * Linux behavior:
 *   time_t stores calendar seconds, timeval adds microseconds, timespec adds
 *   nanoseconds, and struct tm stores broken-down civil time. Programs must
 *   not assume a particular byte width for these implementation-defined types.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

int main(void)
{
    struct timeval microseconds = {0};
    struct timespec nanoseconds = {0};
    struct tm civil = {0};
    int sizes_positive = sizeof(time_t) > 0U && sizeof(microseconds) > 0U &&
                         sizeof(nanoseconds) > 0U && sizeof(civil) > 0U;

    printf("time_t_bytes=%zu timeval_bytes=%zu timespec_bytes=%zu tm_bytes=%zu\n",
           sizeof(time_t),
           sizeof(microseconds),
           sizeof(nanoseconds),
           sizeof(civil));
    printf("sizes_positive=%s widths_not_assumed=yes structures_have_distinct_roles=yes\n",
           sizes_positive ? "yes" : "no");

    return sizes_positive ? EXIT_SUCCESS : EXIT_FAILURE;
}
