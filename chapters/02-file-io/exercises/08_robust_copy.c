/*
 * Exercise 02.08 — Copy a file with robust read/write loops
 *
 * Purpose:
 *   Combine open(), read(), write(), and close() into a binary-safe file copy
 *   that handles short writes, interrupted system calls, EOF, and cleanup.
 *
 * Linux behavior:
 *   read() and write() transfer byte counts rather than C strings. The copy is
 *   therefore valid for text and binary files, including data containing null
 *   bytes. Correctness must follow returned byte counts, not buffer capacity.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum { BUFFER_SIZE = 4096 };

/* Consume every byte produced by one successful read(). */
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
    /* A fixed-size buffer bounds memory use regardless of source-file size. */
    unsigned char buffer[BUFFER_SIZE];
    unsigned long long total = 0;
    int input_fd;
    int output_fd;
    int status = EXIT_FAILURE;

    if (argc != 3) {
        fprintf(stderr, "usage: %s SOURCE DESTINATION\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Source and destination receive separate open-file descriptions and offsets. */
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

    /* Repeat until read() explicitly reports EOF with a zero return value. */
    for (;;) {
        ssize_t bytes_read = read(input_fd, buffer, sizeof(buffer));

        if (bytes_read > 0) {
            /* Never assume one write() can consume the entire read() result. */
            if (write_all(output_fd, buffer, (size_t) bytes_read) == -1) {
                perror("write destination");
                goto cleanup;
            }
            total += (unsigned long long) bytes_read;
        } else if (bytes_read == 0) {
            /* Reaching EOF completes the logical copy successfully. */
            status = EXIT_SUCCESS;
            break;
        } else if (errno != EINTR) {
            perror("read source");
            break;
        }
    }

cleanup:
    /* Cleanup runs for both success and failures after both files are open. */
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
