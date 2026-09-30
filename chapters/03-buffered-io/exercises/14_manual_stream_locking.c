/*
 * Exercise 03.14 — Manually lock a stream for a multi-call transaction
 *
 * Purpose:
 *   Hold a stream lock across several unlocked output operations so the group
 *   forms one logical record from the perspective of cooperating threads.
 *
 * Linux behavior:
 *   Ordinary standard-I/O calls lock the stream one operation at a time. That
 *   protects internal data structures but not a sequence of calls. flockfile()
 *   extends the critical section; unlocked operations avoid redundant locking
 *   while the caller already owns the stream lock.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/manual_locking.txt"

int main(void)
{
    char line[64];
    FILE *stream = fopen(OUTPUT_PATH, "w");

    if (stream == NULL) {
        perror("fopen " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    flockfile(stream);
    if (fputs_unlocked("[", stream) == EOF ||
        fputs_unlocked("one logical record", stream) == EOF ||
        fputs_unlocked("]\n", stream) == EOF) {
        funlockfile(stream);
        perror("fputs_unlocked");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }
    funlockfile(stream);

    if (fclose(stream) == EOF) {
        perror("fclose manual-lock stream");
        return EXIT_FAILURE;
    }

    stream = fopen(OUTPUT_PATH, "r");
    if (stream == NULL) {
        perror("reopen " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fgets(line, sizeof(line), stream) == NULL) {
        if (ferror(stream)) {
            perror("fgets manual-lock record");
        } else {
            fputs("manual-lock record was unexpectedly empty\n", stderr);
        }
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose manual-lock input");
        return EXIT_FAILURE;
    }

    printf("record=%s", line);
    return EXIT_SUCCESS;
}
