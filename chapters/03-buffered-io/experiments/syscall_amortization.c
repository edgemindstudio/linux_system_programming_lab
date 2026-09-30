/*
 * Experiment — Amortize many one-byte operations into fewer system calls
 *
 * Question:
 *   How does user-space buffering change write() traffic when application code
 *   produces one byte at a time?
 *
 * Method:
 *   Run this program once with "raw" and once with "buffered", then compare
 *   `strace -c -e write` results. Both modes produce identical file bytes.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RAW_PATH "build/chapters/03-buffered-io/data/amortization_raw.bin"
#define BUFFERED_PATH "build/chapters/03-buffered-io/data/amortization_buffered.bin"

static int write_one_retry(int fd, unsigned char byte)
{
    ssize_t result;

    do {
        result = write(fd, &byte, 1);
    } while (result == -1 && errno == EINTR);

    return result == 1 ? 0 : -1;
}

static int parse_count(const char *text, size_t *count)
{
    char *end = NULL;
    unsigned long value;

    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value == 0 ||
        value > 1000000UL || value > (unsigned long) SIZE_MAX) {
        return -1;
    }

    *count = (size_t) value;
    return 0;
}

static int run_raw(size_t count)
{
    int fd = open(RAW_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    size_t index;

    if (fd == -1) {
        return -1;
    }

    for (index = 0; index < count; ++index) {
        if (write_one_retry(fd, (unsigned char) 'X') == -1) {
            (void) close(fd);
            return -1;
        }
    }

    return close(fd);
}

static int run_buffered(size_t count)
{
    FILE *stream = fopen(BUFFERED_PATH, "wb");
    size_t index;

    if (stream == NULL) {
        return -1;
    }

    for (index = 0; index < count; ++index) {
        if (fputc('X', stream) == EOF) {
            (void) fclose(stream);
            return -1;
        }
    }

    return fclose(stream) == EOF ? -1 : 0;
}

int main(int argc, char **argv)
{
    size_t count;
    int result;

    if (argc != 3 || parse_count(argv[2], &count) == -1 ||
        (strcmp(argv[1], "raw") != 0 && strcmp(argv[1], "buffered") != 0)) {
        fprintf(stderr, "usage: %s {raw|buffered} BYTE_COUNT\n", argv[0]);
        return EXIT_FAILURE;
    }

    result = strcmp(argv[1], "raw") == 0 ? run_raw(count) : run_buffered(count);
    if (result == -1) {
        perror("produce amortization output");
        return EXIT_FAILURE;
    }

    printf("mode=%s bytes=%zu\n", argv[1], count);
    return EXIT_SUCCESS;
}
