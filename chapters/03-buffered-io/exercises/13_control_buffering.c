/*
 * Exercise 03.13 — Control full, line, and unbuffered output
 *
 * Purpose:
 *   Configure each standard-I/O buffering mode before the first stream
 *   operation and observe when output becomes visible through stat().
 *
 * Linux behavior:
 *   Buffering mode is C-library policy. Full buffering waits for capacity or an
 *   explicit flush, line buffering also flushes when output completes a line,
 *   and unbuffered mode forwards each operation immediately.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FULL_PATH "build/chapters/03-buffered-io/data/full_buffered.txt"
#define LINE_PATH "build/chapters/03-buffered-io/data/line_buffered.txt"
#define NONE_PATH "build/chapters/03-buffered-io/data/unbuffered.txt"

enum { STREAM_BUFFER_SIZE = 4096 };

static int get_size(const char *path, off_t *size)
{
    struct stat info;

    if (stat(path, &info) == -1) {
        return -1;
    }
    *size = info.st_size;
    return 0;
}

static int close_stream(FILE *stream, const char *label)
{
    if (stream != NULL && fclose(stream) == EOF) {
        perror(label);
        return -1;
    }

    return 0;
}

int main(void)
{
    char full_buffer[STREAM_BUFFER_SIZE];
    char line_buffer[STREAM_BUFFER_SIZE];
    FILE *full_stream = NULL;
    FILE *line_stream = NULL;
    FILE *none_stream = NULL;
    off_t full_before;
    off_t full_after;
    off_t line_before;
    off_t line_after;
    off_t none_immediate;
    int status = EXIT_FAILURE;
    int close_failed = 0;

    full_stream = fopen(FULL_PATH, "w");
    line_stream = fopen(LINE_PATH, "w");
    none_stream = fopen(NONE_PATH, "w");
    if (full_stream == NULL || line_stream == NULL || none_stream == NULL) {
        perror("fopen buffering demonstration");
        goto cleanup;
    }

    if (setvbuf(full_stream, full_buffer, _IOFBF, sizeof(full_buffer)) != 0 ||
        setvbuf(line_stream, line_buffer, _IOLBF, sizeof(line_buffer)) != 0 ||
        setvbuf(none_stream, NULL, _IONBF, 0) != 0) {
        fputs("setvbuf failed\n", stderr);
        goto cleanup;
    }

    if (fputs("full", full_stream) == EOF ||
        get_size(FULL_PATH, &full_before) == -1 ||
        fflush(full_stream) == EOF ||
        get_size(FULL_PATH, &full_after) == -1) {
        perror("full-buffered observation");
        goto cleanup;
    }

    if (fputs("line", line_stream) == EOF ||
        get_size(LINE_PATH, &line_before) == -1 ||
        fputc('\n', line_stream) == EOF ||
        get_size(LINE_PATH, &line_after) == -1) {
        perror("line-buffered observation");
        goto cleanup;
    }

    if (fputs("none", none_stream) == EOF ||
        get_size(NONE_PATH, &none_immediate) == -1) {
        perror("unbuffered observation");
        goto cleanup;
    }

    status = EXIT_SUCCESS;

cleanup:
    /* Each stream is closed independently so one failure cannot skip another. */
    close_failed |= close_stream(full_stream, "fclose full-buffered stream") == -1;
    close_failed |= close_stream(line_stream, "fclose line-buffered stream") == -1;
    close_failed |= close_stream(none_stream, "fclose unbuffered stream") == -1;
    if (close_failed) {
        status = EXIT_FAILURE;
    }

    if (status == EXIT_SUCCESS) {
        printf("full: before=%lld after_flush=%lld\n",
               (long long) full_before, (long long) full_after);
        printf("line: before_newline=%lld after_newline=%lld\n",
               (long long) line_before, (long long) line_after);
        printf("none: immediately=%lld\n", (long long) none_immediate);
    }

    return status;
}
