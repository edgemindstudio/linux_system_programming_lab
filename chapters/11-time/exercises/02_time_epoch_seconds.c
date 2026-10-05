/*
 * Exercise 11.02 - Read seconds since the Unix epoch
 *
 * Purpose:
 *   Use time() in both supported forms and confirm that its return value is
 *   also stored through the optional pointer argument.
 *
 * Linux behavior:
 *   time() reports wall-clock seconds since 1970-01-01 00:00:00 UTC. Wall
 *   time can be corrected, so it is appropriate for timestamps but not for
 *   measuring elapsed durations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    time_t stored = (time_t)0;
    time_t returned = time(&stored);

    if (returned == (time_t)-1) {
        perror("time");
        return EXIT_FAILURE;
    }

    printf("epoch_seconds_positive=%s returned_matches_stored=%s\n",
           returned > (time_t)0 ? "yes" : "no",
           returned == stored ? "yes" : "no");
    printf("clock_kind=wall-time resolution=whole-seconds epoch=unix\n");

    return returned == stored ? EXIT_SUCCESS : EXIT_FAILURE;
}
