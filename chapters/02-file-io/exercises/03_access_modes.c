/*
 * Exercise 02.03 — Compare open() access modes
 *
 * Purpose:
 *   Request read-only and write-only access to the same file and observe that
 *   open() validates the requested access against filesystem permissions.
 *
 * Linux behavior:
 *   O_RDONLY, O_WRONLY, and O_RDWR are mutually exclusive access modes. A
 *   successful read-only open does not imply that a write-only open will also
 *   succeed. The kernel performs permission checks when each open() occurs.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    /* /etc/hosts is normally readable by ordinary users. */
    int read_fd = open("/etc/hosts", O_RDONLY);
    int write_fd;

    if (read_fd == -1) {
        perror("open /etc/hosts with O_RDONLY");
        return EXIT_FAILURE;
    }

    printf("Opened /etc/hosts for reading: fd=%d\n", read_fd);

    if (close(read_fd) == -1) {
        perror("close read descriptor");
        return EXIT_FAILURE;
    }

    /* This second call requests a different capability from the kernel. */
    write_fd = open("/etc/hosts", O_WRONLY);
    if (write_fd == -1) {
        perror("O_WRONLY was rejected");
        puts("This is expected for an ordinary user without write permission.");
        return EXIT_SUCCESS;
    }

    printf("O_WRONLY unexpectedly succeeded: fd=%d\n", write_fd);
    /* Avoid altering a system configuration file even when access is granted. */
    puts("No write was attempted. Check the account and file permissions.");

    if (close(write_fd) == -1) {
        perror("close write descriptor");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
