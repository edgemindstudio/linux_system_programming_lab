#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/02-file-io/data/write_demo.txt"

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

int main(void)
{
    const char message[] =
        "write() moves bytes from a user-space buffer to the kernel.\n";
    int fd = open(OUTPUT_PATH,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0644);

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (write_all(fd, message, sizeof(message) - 1) == -1) {
        perror("write " OUTPUT_PATH);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1) {
        perror("close " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("Wrote %zu bytes to %s\n", sizeof(message) - 1, OUTPUT_PATH);
    return EXIT_SUCCESS;
}
