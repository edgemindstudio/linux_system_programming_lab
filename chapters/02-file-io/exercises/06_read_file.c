#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum { BUFFER_SIZE = 256 };

static int write_all(int fd, const void *buffer, size_t count)
{
    const unsigned char *cursor = buffer;

    while (count > 0) {
        ssize_t bytes_written = write(fd, cursor, count);

        if (bytes_written > 0) {
            cursor += bytes_written;
            count -= (size_t) bytes_written;
        } else if (bytes_written == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    unsigned char buffer[BUFFER_SIZE];
    const char *path = argc == 2 ? argv[1] : "/etc/hosts";
    int fd;

    if (argc > 2) {
        fprintf(stderr, "usage: %s [FILE]\n", argv[0]);
        return EXIT_FAILURE;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open input");
        return EXIT_FAILURE;
    }

    for (;;) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));

        if (bytes_read > 0) {
            if (write_all(STDOUT_FILENO,
                          buffer,
                          (size_t) bytes_read) == -1) {
                perror("write stdout");
                (void) close(fd);
                return EXIT_FAILURE;
            }
        } else if (bytes_read == 0) {
            break;
        } else if (errno != EINTR) {
            perror("read input");
            (void) close(fd);
            return EXIT_FAILURE;
        }
    }

    if (close(fd) == -1) {
        perror("close input");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
