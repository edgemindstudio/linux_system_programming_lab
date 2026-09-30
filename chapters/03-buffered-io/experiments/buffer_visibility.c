/*
 * Experiment — Visibility of bytes held in a FILE buffer
 *
 * Question:
 *   Can a second descriptor read bytes that fputs() has placed only in a
 *   full user-space output buffer?
 *
 * Prediction:
 *   Before fflush(), the independent reader reaches kernel-visible EOF. After
 *   fflush(), the same reader can obtain the bytes from the file.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/buffer_visibility.txt"

enum { STREAM_BUFFER_SIZE = 4096 };

static ssize_t read_retry(int fd, void *buffer, size_t count)
{
    ssize_t result;

    do {
        result = read(fd, buffer, count);
    } while (result == -1 && errno == EINTR);

    return result;
}

int main(void)
{
    const char message[] = "hidden\n";
    char stream_buffer[STREAM_BUFFER_SIZE];
    char observed[sizeof(message)] = {0};
    FILE *writer = fopen(OUTPUT_PATH, "w");
    int reader_fd;
    ssize_t before_flush;
    ssize_t after_flush;
    int close_failed = 0;

    if (writer == NULL) {
        perror("fopen " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (setvbuf(writer, stream_buffer, _IOFBF, sizeof(stream_buffer)) != 0 ||
        fputs(message, writer) == EOF) {
        fputs("failed to prepare buffered writer\n", stderr);
        (void) fclose(writer);
        return EXIT_FAILURE;
    }

    reader_fd = open(OUTPUT_PATH, O_RDONLY);
    if (reader_fd == -1) {
        perror("open independent reader");
        (void) fclose(writer);
        return EXIT_FAILURE;
    }

    before_flush = read_retry(reader_fd, observed, sizeof(observed) - 1);
    if (before_flush != 0) {
        fputs("expected kernel-visible EOF before fflush\n", stderr);
        (void) close(reader_fd);
        (void) fclose(writer);
        return EXIT_FAILURE;
    }

    if (fflush(writer) == EOF) {
        perror("fflush writer");
        (void) close(reader_fd);
        (void) fclose(writer);
        return EXIT_FAILURE;
    }

    after_flush = read_retry(reader_fd, observed, sizeof(observed) - 1);
    if (after_flush != (ssize_t) (sizeof(message) - 1)) {
        if (after_flush == -1) {
            perror("read after fflush");
        } else {
            fprintf(stderr,
                    "short read after fflush: expected %zu, received %zd\n",
                    sizeof(message) - 1,
                    after_flush);
        }
        (void) close(reader_fd);
        (void) fclose(writer);
        return EXIT_FAILURE;
    }

    if (close(reader_fd) == -1) {
        perror("close visibility reader");
        close_failed = 1;
    }
    if (fclose(writer) == EOF) {
        perror("fclose visibility writer");
        close_failed = 1;
    }
    if (close_failed) {
        return EXIT_FAILURE;
    }

    printf("separate reader: before_flush=%zd after_flush=%zd data=%s",
           before_flush,
           after_flush,
           observed);
    return EXIT_SUCCESS;
}
