#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DATA_DIRECTORY "build/chapter02/data"
#define OUTPUT_PATH DATA_DIRECTORY "/durable_record.txt"

static int write_all(int fd, const void *buffer, size_t count)
{
    const char *cursor = buffer;

    while (count > 0) {
        ssize_t written = write(fd, cursor, count);

        if (written > 0) {
            cursor += written;
            count -= (size_t) written;
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
    const char first[] = "payload synchronized with fdatasync()\n";
    const char second[] = "payload synchronized with fsync()\n";
    int fd;
    int directory_fd;

    fd = open(OUTPUT_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (write_all(fd, first, sizeof(first) - 1) == -1) {
        perror("write first record");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    puts("write() returned: bytes reached the kernel, not necessarily storage.");

    if (fdatasync(fd) == -1) {
        perror("fdatasync");
        (void) close(fd);
        return EXIT_FAILURE;
    }
    puts("fdatasync() completed: file data and required metadata were requested to storage.");

    if (write_all(fd, second, sizeof(second) - 1) == -1) {
        perror("write second record");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (fsync(fd) == -1) {
        perror("fsync file");
        (void) close(fd);
        return EXIT_FAILURE;
    }
    puts("fsync() completed: file data and associated metadata were requested to storage.");

    if (close(fd) == -1) {
        perror("close " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    directory_fd = open(DATA_DIRECTORY, O_RDONLY | O_DIRECTORY);
    if (directory_fd == -1) {
        perror("open " DATA_DIRECTORY);
        return EXIT_FAILURE;
    }

    if (fsync(directory_fd) == -1) {
        fprintf(stderr,
                "directory fsync is unsupported here: %s\n",
                strerror(errno));
    } else {
        puts("fsync() completed for the containing directory entry.");
    }

    if (close(directory_fd) == -1) {
        perror("close " DATA_DIRECTORY);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
