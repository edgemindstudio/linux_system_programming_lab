/*
 * Exercise 03.02 — Open streams with fopen() modes
 *
 * Purpose:
 *   Use update, append, and read modes while observing how each mode controls
 *   creation, truncation, positioning, and permitted operations.
 *
 * Linux behavior:
 *   fopen() translates its mode string into an appropriate descriptor open and
 *   then constructs a FILE stream around that descriptor. When a stream permits
 *   both reading and writing, a flush or positioning operation is required at
 *   the appropriate direction change so buffered state remains coherent.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/fopen_modes.txt"

static int checked_close(FILE *stream, const char *label)
{
    if (fclose(stream) == EOF) {
        perror(label);
        return -1;
    }

    return 0;
}

int main(void)
{
    char line[64];
    FILE *stream = fopen(OUTPUT_PATH, "w+");

    if (stream == NULL) {
        perror("fopen w+ " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    /* "w+" creates or truncates the file and permits both output and input. */
    if (fputs("alpha\n", stream) == EOF) {
        perror("fputs alpha");
        (void) checked_close(stream, "fclose after write failure");
        return EXIT_FAILURE;
    }

    /* Flush output, then reposition before changing this update stream to input. */
    if (fflush(stream) == EOF || fseeko(stream, 0, SEEK_SET) == -1) {
        perror("prepare w+ stream for reading");
        (void) checked_close(stream, "fclose w+ stream");
        return EXIT_FAILURE;
    }

    if (fgets(line, sizeof(line), stream) == NULL) {
        if (ferror(stream)) {
            perror("fgets from w+ stream");
        } else {
            fputs("unexpected EOF in w+ stream\n", stderr);
        }
        (void) checked_close(stream, "fclose w+ stream");
        return EXIT_FAILURE;
    }

    if (checked_close(stream, "fclose w+ stream") == -1) {
        return EXIT_FAILURE;
    }

    /* "a" preserves existing bytes and forces each write to the current end. */
    stream = fopen(OUTPUT_PATH, "a");
    if (stream == NULL) {
        perror("fopen a " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fputs("beta\n", stream) == EOF) {
        perror("fputs beta");
        (void) checked_close(stream, "fclose append stream");
        return EXIT_FAILURE;
    }

    /* fclose() is checked separately because it can report a flush failure. */
    if (checked_close(stream, "fclose append stream") == -1) {
        return EXIT_FAILURE;
    }

    stream = fopen(OUTPUT_PATH, "r");
    if (stream == NULL) {
        perror("fopen r " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    puts("contents after w+ then a:");
    while (fgets(line, sizeof(line), stream) != NULL) {
        if (fputs(line, stdout) == EOF) {
            perror("fputs stdout");
            (void) checked_close(stream, "fclose read stream");
            return EXIT_FAILURE;
        }
    }

    if (ferror(stream)) {
        perror("read " OUTPUT_PATH);
        (void) checked_close(stream, "fclose read stream");
        return EXIT_FAILURE;
    }

    if (checked_close(stream, "fclose read stream") == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
