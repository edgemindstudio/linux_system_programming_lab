/*
 * Exercise 03.12 — Distinguish EOF from a stream error
 *
 * Purpose:
 *   Produce one ordinary end-of-file condition and one Linux write error, then
 *   inspect and clear the stream indicators.
 *
 * Linux behavior:
 *   Many input functions use the same sentinel for EOF and failure. feof() and
 *   ferror() identify which condition occurred. Indicators remain set until
 *   clearerr(), rewind(), or another specified operation clears them.
 */

#include <stdio.h>
#include <stdlib.h>

#define INPUT_PATH "build/chapters/03-buffered-io/data/eof_state.txt"

int main(void)
{
    FILE *stream = fopen(INPUT_PATH, "w+");
    FILE *full_device;
    int first;
    int second;
    int failed_write;

    if (stream == NULL) {
        perror("fopen " INPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fputc('X', stream) == EOF || fflush(stream) == EOF) {
        perror("initialize EOF stream");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }
    rewind(stream);

    first = fgetc(stream);
    second = fgetc(stream);
    if (first == EOF || second != EOF) {
        fputs("failed to create expected EOF state\n", stderr);
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    printf("EOF state: feof=%d ferror=%d\n",
           feof(stream) != 0,
           ferror(stream) != 0);
    clearerr(stream);
    printf("after clearerr: feof=%d ferror=%d\n",
           feof(stream) != 0,
           ferror(stream) != 0);

    if (fclose(stream) == EOF) {
        perror("fclose EOF stream");
        return EXIT_FAILURE;
    }

    /* /dev/full is a Linux device that rejects writes with ENOSPC. */
    full_device = fopen("/dev/full", "w");
    if (full_device == NULL) {
        perror("fopen /dev/full");
        return EXIT_FAILURE;
    }

    if (setvbuf(full_device, NULL, _IONBF, 0) != 0) {
        fputs("setvbuf /dev/full failed\n", stderr);
        (void) fclose(full_device);
        return EXIT_FAILURE;
    }

    failed_write = fputc('X', full_device);
    if (failed_write != EOF || !ferror(full_device)) {
        fputs("failed to create expected stream error\n", stderr);
        (void) fclose(full_device);
        return EXIT_FAILURE;
    }

    printf("error state: feof=%d ferror=%d\n",
           feof(full_device) != 0,
           ferror(full_device) != 0);
    clearerr(full_device);

    if (fclose(full_device) == EOF) {
        perror("fclose /dev/full");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
