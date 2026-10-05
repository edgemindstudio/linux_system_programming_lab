/*
 * Exercise 11.03 - Read wall time with microsecond fields
 *
 * Purpose:
 *   Retrieve wall time through gettimeofday() and validate the invariant on
 *   the microsecond component.
 *
 * Linux behavior:
 *   timeval can express microseconds, but the number of digits in the
 *   interface does not guarantee equal hardware or kernel clock precision.
 *   The obsolete timezone argument should be NULL.
 */

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(void)
{
    struct timeval now;
    int microseconds_valid;

    if (gettimeofday(&now, NULL) == -1) {
        perror("gettimeofday");
        return EXIT_FAILURE;
    }

    microseconds_valid = now.tv_usec >= 0 && now.tv_usec < 1000000;
    printf("seconds_positive=%s microseconds_in_range=%s\n",
           now.tv_sec > 0 ? "yes" : "no",
           microseconds_valid ? "yes" : "no");
    printf("timezone_argument=null expressed_precision=microseconds\n");

    return microseconds_valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
