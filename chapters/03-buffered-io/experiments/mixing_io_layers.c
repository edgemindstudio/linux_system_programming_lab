/*
 * Experiment — Mix standard I/O and descriptor I/O on one open file
 *
 * Question:
 *   What can happen when fputs() has pending bytes but write() uses the stream's
 *   descriptor directly without first synchronizing the two I/O layers?
 *
 * Warning:
 *   The first case intentionally violates POSIX active-handle coordination and
 *   is observational, not portable. The second case uses fflush() before the
 *   descriptor-level write and produces a defined ordering.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UNSAFE_PATH "build/chapters/03-buffered-io/data/mixed_unsafe.txt"
#define SAFE_PATH "build/chapters/03-buffered-io/data/mixed_safe.txt"

enum { STREAM_BUFFER_SIZE = 128 };

static int direct_write_all(int fd, const char *text, size_t count)
{
    size_t offset = 0;

    while (offset < count) {
        ssize_t written = write(fd, text + offset, count - offset);

        if (written > 0) {
            offset += (size_t) written;
        } else if (written == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }

    return 0;
}

static int produce_case(const char *path, int synchronize)
{
    char buffer[STREAM_BUFFER_SIZE];
    FILE *stream = fopen(path, "w");
    int fd;

    if (stream == NULL) {
        return -1;
    }

    if (setvbuf(stream, buffer, _IOFBF, sizeof(buffer)) != 0) {
        (void) fclose(stream);
        return -1;
    }

    fd = fileno(stream);
    if (fd == -1 || fputs("ABC", stream) == EOF) {
        (void) fclose(stream);
        return -1;
    }

    if (synchronize && fflush(stream) == EOF) {
        (void) fclose(stream);
        return -1;
    }

    if (direct_write_all(fd, "raw", 3) == -1) {
        (void) fclose(stream);
        return -1;
    }

    if (fclose(stream) == EOF) {
        return -1;
    }

    return 0;
}

static int read_text(const char *path, char *buffer, size_t capacity)
{
    FILE *stream = fopen(path, "r");

    if (stream == NULL) {
        return -1;
    }

    if (fgets(buffer, (int) capacity, stream) == NULL && ferror(stream)) {
        (void) fclose(stream);
        return -1;
    }

    return fclose(stream) == EOF ? -1 : 0;
}

int main(void)
{
    char unsafe_text[16] = {0};
    char safe_text[16] = {0};

    if (produce_case(UNSAFE_PATH, 0) == -1 ||
        produce_case(SAFE_PATH, 1) == -1 ||
        read_text(UNSAFE_PATH, unsafe_text, sizeof(unsafe_text)) == -1 ||
        read_text(SAFE_PATH, safe_text, sizeof(safe_text)) == -1) {
        perror("mixed-I/O experiment");
        return EXIT_FAILURE;
    }

    printf("without coordination (nonportable observation): %s\n", unsafe_text);
    printf("after fflush (defined ordering): %s\n", safe_text);

    return strcmp(safe_text, "ABCraw") == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
