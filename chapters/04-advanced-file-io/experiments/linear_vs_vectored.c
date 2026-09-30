/*
 * Experiment - Linear writes versus one vectored write
 *
 * Question:
 *   Can identical file bytes require different numbers of write syscalls?
 *
 * Method:
 *   Run once with "linear" and once with "vectored", then compare both output
 *   files and inspect `strace -c -e write,writev` for each mode.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#define LINEAR_PATH "build/chapters/04-advanced-file-io/data/linear_output.txt"
#define VECTORED_PATH "build/chapters/04-advanced-file-io/data/vectored_output.txt"

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

int main(int argc, char **argv)
{
    static char first[] = "alpha|";
    static char second[] = "beta|";
    static char third[] = "gamma\n";
    struct iovec vector[] = {
        { .iov_base = first, .iov_len = sizeof(first) - 1 },
        { .iov_base = second, .iov_len = sizeof(second) - 1 },
        { .iov_base = third, .iov_len = sizeof(third) - 1 }
    };
    const size_t expected = sizeof(first) + sizeof(second) + sizeof(third) - 3;
    const char *path;
    int fd;
    int result = 0;

    if (argc != 2 ||
        (strcmp(argv[1], "linear") != 0 &&
         strcmp(argv[1], "vectored") != 0)) {
        fprintf(stderr, "usage: %s {linear|vectored}\n", argv[0]);
        return EXIT_FAILURE;
    }

    path = strcmp(argv[1], "linear") == 0 ? LINEAR_PATH : VECTORED_PATH;
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open experiment output");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "linear") == 0) {
        result |= write_all(fd, first, sizeof(first) - 1);
        result |= write_all(fd, second, sizeof(second) - 1);
        result |= write_all(fd, third, sizeof(third) - 1);
    } else {
        ssize_t written;

        do {
            written = writev(fd, vector, 3);
        } while (written == -1 && errno == EINTR);
        result = written == (ssize_t) expected ? 0 : -1;
    }

    if (result == -1) {
        perror("produce experiment output");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1) {
        perror("close experiment output");
        return EXIT_FAILURE;
    }

    printf("mode=%s bytes=%zu\n", argv[1], expected);
    return EXIT_SUCCESS;
}
