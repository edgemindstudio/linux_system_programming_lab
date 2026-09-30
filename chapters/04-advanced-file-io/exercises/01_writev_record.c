/*
 * Exercise 04.01 - Gather several buffers with writev()
 *
 * Purpose:
 *   Write one logical record whose fields live in separate memory buffers.
 *   A single writev() request gathers those buffers in vector order.
 *
 * Linux behavior:
 *   writev() returns a byte count for the whole vector, not a segment count.
 *   A successful call may still be short, so writev_all() advances across
 *   complete and partially consumed iovec entries before trying again.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/04-advanced-file-io/data/writev_record.txt"

static ssize_t writev_all(int fd, struct iovec *vector, int count)
{
    struct iovec *current = vector;
    int remaining = count;
    ssize_t total = 0;

    while (remaining > 0) {
        ssize_t written = writev(fd, current, remaining);
        size_t consumed;

        if (written == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (written == 0) {
            errno = EIO;
            return -1;
        }

        total += written;
        consumed = (size_t) written;

        while (remaining > 0 && consumed >= current[0].iov_len) {
            consumed -= current[0].iov_len;
            ++current;
            --remaining;
        }

        if (remaining > 0 && consumed > 0) {
            unsigned char *base = current[0].iov_base;

            current[0].iov_base = base + consumed;
            current[0].iov_len -= consumed;
        }
    }

    return total;
}

int main(void)
{
    char first[] = "type=event ";
    char second[] = "value=42 ";
    char third[] = "status=ok\n";
    struct iovec vector[] = {
        { .iov_base = first, .iov_len = sizeof(first) - 1 },
        { .iov_base = second, .iov_len = sizeof(second) - 1 },
        { .iov_base = third, .iov_len = sizeof(third) - 1 }
    };
    const int count = (int) (sizeof(vector) / sizeof(vector[0]));
    const size_t expected = sizeof(first) + sizeof(second) + sizeof(third) - 3;
    int fd = open(OUTPUT_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ssize_t written;

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    written = writev_all(fd, vector, count);
    if (written == -1) {
        perror("writev");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1) {
        perror("close " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if ((size_t) written != expected) {
        fprintf(stderr, "expected %zu bytes, wrote %zd\n", expected, written);
        return EXIT_FAILURE;
    }

    printf("segments=%d bytes=%zd record=%s%s%s",
           count, written, first, second, third);
    return EXIT_SUCCESS;
}
