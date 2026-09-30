/*
 * Exercise 03.11 — Flush pending output explicitly
 *
 * Purpose:
 *   Keep a small write in a full user-space buffer, inspect the kernel-visible
 *   file size, call fflush(), and inspect the size again.
 *
 * Linux behavior:
 *   fflush() asks the C library to pass pending stream output to the underlying
 *   descriptor. It does not provide storage durability; fsync() or fdatasync()
 *   is a separate request concerning kernel state and persistent storage.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/flush_stream.txt"

enum { STREAM_BUFFER_SIZE = 4096 };

static int file_size(const char *path, off_t *size)
{
    struct stat info;

    if (stat(path, &info) == -1) {
        return -1;
    }
    *size = info.st_size;
    return 0;
}

int main(void)
{
    const char message[] = "pending-user-buffer";
    char stream_buffer[STREAM_BUFFER_SIZE];
    FILE *stream = fopen(OUTPUT_PATH, "w");
    off_t before_flush;
    off_t after_flush;

    if (stream == NULL) {
        perror("fopen " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    /* Buffer control must occur before any other operation on this stream. */
    if (setvbuf(stream, stream_buffer, _IOFBF, sizeof(stream_buffer)) != 0) {
        fputs("setvbuf failed\n", stderr);
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fputs(message, stream) == EOF ||
        file_size(OUTPUT_PATH, &before_flush) == -1) {
        perror("buffer message or stat before fflush");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fflush(stream) == EOF ||
        file_size(OUTPUT_PATH, &after_flush) == -1) {
        perror("fflush or stat after fflush");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("kernel-visible size: before fflush=%lld after fflush=%lld\n",
           (long long) before_flush,
           (long long) after_flush);
    return EXIT_SUCCESS;
}
