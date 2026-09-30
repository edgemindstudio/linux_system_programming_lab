/*
 * Exercise 03.09 — Copy a file with fread() and fwrite()
 *
 * Purpose:
 *   Build a binary-safe copy program using standard I/O while handling partial
 *   output, EOF, stream errors, close-time failures, and cleanup.
 *
 * Linux behavior:
 *   fread() may satisfy many application reads from one C-library buffer fill,
 *   and fwrite() may combine many application writes before calling write().
 *   Correctness still depends on returned element counts and stream indicators.
 */

#include <stdio.h>
#include <stdlib.h>

enum { BUFFER_SIZE = 4096 };

/* Write every byte from one successful fread() result. */
static int fwrite_all(FILE *stream, const unsigned char *buffer, size_t count)
{
    size_t offset = 0;

    while (offset < count) {
        size_t written = fwrite(buffer + offset, 1, count - offset, stream);

        if (written == 0) {
            return -1;
        }
        offset += written;
    }

    return 0;
}

int main(int argc, char **argv)
{
    unsigned char buffer[BUFFER_SIZE];
    unsigned long long total = 0;
    FILE *input;
    FILE *output;
    int status = EXIT_FAILURE;

    if (argc != 3) {
        fprintf(stderr, "usage: %s SOURCE DESTINATION\n", argv[0]);
        return EXIT_FAILURE;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("fopen source");
        return EXIT_FAILURE;
    }

    output = fopen(argv[2], "wb");
    if (output == NULL) {
        perror("fopen destination");
        (void) fclose(input);
        return EXIT_FAILURE;
    }

    for (;;) {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), input);

        if (bytes_read > 0) {
            if (fwrite_all(output, buffer, bytes_read) == -1) {
                perror("fwrite destination");
                goto cleanup;
            }
            total += (unsigned long long) bytes_read;
        }

        if (bytes_read < sizeof(buffer)) {
            if (ferror(input)) {
                perror("fread source");
                goto cleanup;
            }
            if (feof(input)) {
                status = EXIT_SUCCESS;
                break;
            }
        }
    }

cleanup:
    if (fclose(input) == EOF) {
        perror("fclose source");
        status = EXIT_FAILURE;
    }

    /* fclose() is part of output correctness because it flushes pending bytes. */
    if (fclose(output) == EOF) {
        perror("fclose destination");
        status = EXIT_FAILURE;
    }

    if (status == EXIT_SUCCESS) {
        printf("copied %llu bytes with standard I/O\n", total);
    }

    return status;
}
