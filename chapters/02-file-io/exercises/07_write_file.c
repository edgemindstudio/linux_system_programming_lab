/*
 * Exercise 02.07 — Write a complete user-space buffer
 *
 * Purpose:
 *   Create a file and reliably transfer every byte of a C array to it with
 *   write(), including correct handling of short and interrupted writes.
 *
 * Linux behavior:
 *   A successful write() reports how many bytes the kernel accepted; that
 *   number can be smaller than requested. Success transfers data into kernel
 *   state, but does not by itself guarantee durable storage on the device.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/02-file-io/data/write_demo.txt"

/* Keep writing the unwritten suffix until the complete logical record is sent. */
static int write_all(int fd, const void *buffer, size_t count)
{
    const unsigned char *cursor = buffer;

    while (count > 0) {
        ssize_t bytes_written = write(fd, cursor, count);

        if (bytes_written > 0) {
            /* Continue at the first byte that the previous call did not accept. */
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

    /* O_TRUNC makes repeated runs deterministic by replacing old contents. */
    int fd = open(OUTPUT_PATH,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0644);

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    /* Exclude the C string's terminating null byte from the file. */
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
