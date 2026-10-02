/*
 * Exercise 08.16 - Read randomness through a character device
 *
 * Purpose:
 *   Inspect /dev/urandom as a device node and read bytes without treating it
 *   like a finite regular file.
 *
 * Linux behavior:
 *   The pathname names a character device. read() is serviced by a kernel
 *   driver rather than by fetching stored bytes from a regular-file inode.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    unsigned char bytes[32];
    struct stat status;
    size_t total = 0U;
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);

    if (fd == -1) {
        perror("open /dev/urandom");
        return EXIT_FAILURE;
    }
    if (fstat(fd, &status) == -1) {
        perror("fstat /dev/urandom");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    while (total < sizeof(bytes)) {
        ssize_t received = read(fd, bytes + total, sizeof(bytes) - total);

        if (received == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read /dev/urandom");
            (void)close(fd);
            return EXIT_FAILURE;
        }
        if (received == 0) {
            fputs("unexpected EOF from /dev/urandom\n", stderr);
            (void)close(fd);
            return EXIT_FAILURE;
        }
        total += (size_t)received;
    }
    if (close(fd) == -1) {
        perror("close /dev/urandom");
        return EXIT_FAILURE;
    }

    printf("character_device=%s bytes_read=%zu\n",
           S_ISCHR(status.st_mode) ? "yes" : "no",
           total);
    printf("random_values_not_printed=yes driver_served_read=yes\n");

    return EXIT_SUCCESS;
}
