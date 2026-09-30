/*
 * Exercise 03.01 — Standard streams and their descriptors
 *
 * Purpose:
 *   Connect the C library's stdin, stdout, and stderr FILE streams to the
 *   standard file descriptors introduced in Chapter 2.
 *
 * Linux behavior:
 *   A FILE object is user-space state managed by the C library. fileno()
 *   exposes the underlying descriptor used when that stream eventually asks
 *   the kernel to perform I/O. The stream adds buffering, indicators, locking,
 *   and conversion rules around the descriptor; it does not replace it.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int input_fd = fileno(stdin);
    int output_fd = fileno(stdout);
    int error_fd = fileno(stderr);

    if (input_fd == -1 || output_fd == -1 || error_fd == -1) {
        perror("fileno");
        return EXIT_FAILURE;
    }

    printf("stdin  -> fd %d (expected %d)\n", input_fd, STDIN_FILENO);
    printf("stdout -> fd %d (expected %d)\n", output_fd, STDOUT_FILENO);
    printf("stderr -> fd %d (expected %d)\n", error_fd, STDERR_FILENO);

    return EXIT_SUCCESS;
}
