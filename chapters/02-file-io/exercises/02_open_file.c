/*
 * Exercise 02.02 — Open and close a file
 *
 * Purpose:
 *   Open /etc/hosts for reading, inspect the returned file descriptor, and
 *   release that descriptor with close().
 *
 * Linux behavior:
 *   open() asks the kernel to create an open-file description and returns a
 *   process-local descriptor that refers to it. The descriptor is a small
 *   integer handle, not the file itself. A return value of -1 reports failure
 *   and leaves errno describing the reason.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    /* O_RDONLY requests read access without modifying the file. */
    int fd = open("/etc/hosts", O_RDONLY);

    if (fd == -1) {
        /* perror() combines this label with the message represented by errno. */
        perror("open /etc/hosts");
        return EXIT_FAILURE;
    }

    printf("open() returned file descriptor %d\n", fd);

    /* Every successfully opened descriptor should be closed when no longer used. */
    if (close(fd) == -1) {
        perror("close /etc/hosts");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
