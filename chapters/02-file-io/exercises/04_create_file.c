/*
 * Exercise 02.04 — Create or truncate a file with open()
 *
 * Purpose:
 *   Combine open() flags to obtain a write-only descriptor, create the file
 *   when necessary, and make an existing file empty.
 *
 * Linux behavior:
 *   O_CREAT makes the third open() argument meaningful. The requested mode is
 *   filtered by the process umask. O_TRUNC sets an existing regular file's
 *   length to zero after the open succeeds.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/02-file-io/data/love_demo.txt"

int main(void)
{
    /*
     * 0664 requests rw-rw-r-- before the process umask removes permissions.
     * The Makefile creates the parent build data directory before this runs.
     */
    int fd = open(OUTPUT_PATH,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0664);

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("Created or opened %s with fd=%d\n", OUTPUT_PATH, fd);

    if (close(fd) == -1) {
        perror("close " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
