/*
 * Exercise 11.12 - Round-trip civil time through time_t
 *
 * Purpose:
 *   Convert a broken-down UTC value to epoch time with mktime() and back with
 *   gmtime_r() under an explicit deterministic timezone.
 *
 * Linux behavior:
 *   mktime() interprets struct tm as local civil time and normalizes its
 *   fields. Setting TZ=UTC0 makes the local policy explicit for this process.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    struct tm input = {
        .tm_sec = 5,
        .tm_min = 4,
        .tm_hour = 3,
        .tm_mday = 2,
        .tm_mon = 0,
        .tm_year = 126,
        .tm_isdst = -1,
    };
    struct tm output;
    time_t encoded;
    int matches;

    if (setenv("TZ", "UTC0", 1) == -1) {
        perror("setenv TZ");
        return EXIT_FAILURE;
    }
    tzset();
    encoded = mktime(&input);
    if (encoded == (time_t)-1 || gmtime_r(&encoded, &output) == NULL) {
        fputs("time round-trip failed\n", stderr);
        return EXIT_FAILURE;
    }

    matches = output.tm_year == 126 && output.tm_mon == 0 &&
              output.tm_mday == 2 && output.tm_hour == 3 &&
              output.tm_min == 4 && output.tm_sec == 5;
    printf("mktime_succeeded=yes roundtrip_matches=%s normalized_month=%d\n",
           matches ? "yes" : "no",
           output.tm_mon + 1);
    printf("timezone_explicit=yes tm_isdst_auto_detected=yes\n");

    return matches ? EXIT_SUCCESS : EXIT_FAILURE;
}
