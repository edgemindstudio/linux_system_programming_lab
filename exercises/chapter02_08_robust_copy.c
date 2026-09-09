#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum { BUFFER_SIZE = 4096 };

static int write_all(int fd, const void *buffer, size_t count)
{
    const unsigned char *cursor = buffer;

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

int main(int argc, char **argv)
{
    unsigned char buffer[BUFFER_SIZE];
    unsigned long long total = 0;
    int input_fd;
    int output_fd;
    int status = EXIT_FAILURE;

    if (argc != 3) {
        fprintf(stderr, "usage: %s SOURCE DESTINATION\n", argv[0]);
        return EXIT_FAILURE;
    }

    input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
        perror("open source");
        return EXIT_FAILURE;
    }

    output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("open destination");
        (void) close(input_fd);
        return EXIT_FAILURE;
    }

    for (;;) {
        ssize_t bytes_read = read(input_fd, buffer, sizeof(buffer));

        if (bytes_read > 0) {
            if (write_all(output_fd, buffer, (size_t) bytes_read) == -1) {
                perror("write destination");
                goto cleanup;
            }
            total += (unsigned long long) bytes_read;
        } else if (bytes_read == 0) {
            status = EXIT_SUCCESS;
            break;
        } else if (errno != EINTR) {
            perror("read source");
            break;
        }
    }

cleanup:
    if (close(input_fd) == -1) {
        perror("close source");
        status = EXIT_FAILURE;
    }

    if (close(output_fd) == -1) {
        perror("close destination");
        status = EXIT_FAILURE;
    }

    if (status == EXIT_SUCCESS) {
        printf("copied %llu bytes\n", total);
    }

    return status;
}
