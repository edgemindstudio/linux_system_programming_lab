/*
 * Exercise 02.05 — Compare creat() with open()
 *
 * Purpose:
 *   Demonstrate that creat(path, mode) is the historical shorthand for
 *   open(path, O_WRONLY | O_CREAT | O_TRUNC, mode).
 *
 * Linux behavior:
 *   Both calls return independent descriptors and both requested modes are
 *   filtered by the process umask. Descriptor values identify entries in this
 *   process's descriptor table; matching behavior does not require matching
 *   descriptor numbers.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CREAT_PATH "build/chapters/02-file-io/data/created_with_creat.txt"
#define OPEN_PATH "build/chapters/02-file-io/data/created_with_open.txt"
#define CREATION_MODE 0644

/* Centralize close() checking so every successful open has a checked cleanup. */
static int checked_close(int fd, const char *label)
{
    if (close(fd) == -1) {
        perror(label);
        return -1;
    }

    return 0;
}

int main(void)
{
    /* creat() always requests write-only, create-if-needed, truncating access. */
    int creat_fd = creat(CREAT_PATH, CREATION_MODE);
    int open_fd;

    if (creat_fd == -1) {
        perror("creat " CREAT_PATH);
        return EXIT_FAILURE;
    }

    /* Spell out the flags that are equivalent to creat(). */
    open_fd = open(OPEN_PATH,
                   O_WRONLY | O_CREAT | O_TRUNC,
                   CREATION_MODE);
    if (open_fd == -1) {
        perror("open " OPEN_PATH);
        /* Preserve the original open() failure while still releasing creat_fd. */
        (void) checked_close(creat_fd, "close creat descriptor");
        return EXIT_FAILURE;
    }

    printf("creat() fd = %d\n", creat_fd);
    printf("open() fd  = %d\n", open_fd);
    puts("Both files used the same requested mode and truncating write-only behavior.");

    if (checked_close(creat_fd, "close creat descriptor") == -1) {
        (void) checked_close(open_fd, "close open descriptor");
        return EXIT_FAILURE;
    }

    if (checked_close(open_fd, "close open descriptor") == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
