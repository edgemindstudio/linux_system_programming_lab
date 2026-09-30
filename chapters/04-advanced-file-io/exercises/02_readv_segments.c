/*
 * Exercise 04.02 - Scatter one file read into several buffers
 *
 * Purpose:
 *   Recover a fixed-layout record with readv(), placing its header, payload,
 *   and trailer into independent arrays in a single request.
 *
 * Linux behavior:
 *   readv() fills iov[0] before iov[1], then continues in vector order. Its
 *   return value is the total bytes placed across every segment. The arrays
 *   reserve their own extra byte for C string termination.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>

#define INPUT_PATH "build/chapters/04-advanced-file-io/data/readv_record.bin"

static int write_all(int fd, const void *buffer, size_t count)
{
    const unsigned char *bytes = buffer;
    size_t offset = 0;

    while (offset < count) {
        ssize_t written = write(fd, bytes + offset, count - offset);

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

int main(void)
{
    static const char encoded[] = "HDR1payload-dataEND";
    char header[5] = {0};
    char payload[13] = {0};
    char trailer[4] = {0};
    struct iovec vector[] = {
        { .iov_base = header, .iov_len = sizeof(header) - 1 },
        { .iov_base = payload, .iov_len = sizeof(payload) - 1 },
        { .iov_base = trailer, .iov_len = sizeof(trailer) - 1 }
    };
    const int count = (int) (sizeof(vector) / sizeof(vector[0]));
    const size_t expected = sizeof(encoded) - 1;
    int fd = open(INPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    ssize_t received;

    if (fd == -1) {
        perror("open " INPUT_PATH);
        return EXIT_FAILURE;
    }

    if (write_all(fd, encoded, expected) == -1 ||
        lseek(fd, 0, SEEK_SET) == (off_t) -1) {
        perror("initialize readv input");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    do {
        received = readv(fd, vector, count);
    } while (received == -1 && errno == EINTR);

    if (received == -1) {
        perror("readv");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1) {
        perror("close " INPUT_PATH);
        return EXIT_FAILURE;
    }

    if ((size_t) received != expected) {
        fprintf(stderr, "expected %zu bytes, received %zd\n",
                expected, received);
        return EXIT_FAILURE;
    }

    printf("bytes=%zd header=%s payload=%s trailer=%s\n",
           received, header, payload, trailer);
    return EXIT_SUCCESS;
}
