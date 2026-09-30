/*
 * Exercise 04.08 - Persist a MAP_SHARED modification
 *
 * Purpose:
 *   Change file-backed bytes through memory and explicitly synchronize them.
 *
 * Linux behavior:
 *   MAP_SHARED connects dirty mapping pages to the backing file. msync() with
 *   MS_SYNC waits for the requested range to be synchronized before returning.
 *   This is the mapping-oriented counterpart to explicit file synchronization.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/04-advanced-file-io/data/mmap_shared.txt"

static int write_all(int fd, const void *buffer, size_t count)
{
    const unsigned char *bytes = buffer;
    size_t offset = 0;

    while (offset < count) {
        ssize_t written = write(fd, bytes + offset, count - offset);

        if (written > 0) {
            offset += (size_t) written;
        } else if (written == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }
    return 0;
}

int main(void)
{
    static const char initial[] = "original\n";
    static const char replacement[] = "shared!!";
    char verification[sizeof(initial)] = {0};
    char mapped_copy[sizeof(initial)] = {0};
    int fd = open(OUTPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    char *mapping;

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (write_all(fd, initial, sizeof(initial) - 1) == -1) {
        perror("initialize shared mapping file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    mapping = mmap(NULL,
                   sizeof(initial) - 1,
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED,
                   fd,
                   0);
    if (mapping == MAP_FAILED) {
        perror("mmap MAP_SHARED");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    memcpy(mapping, replacement, sizeof(replacement) - 1);
    memcpy(mapped_copy, mapping, sizeof(initial) - 1);

    if (msync(mapping, sizeof(initial) - 1, MS_SYNC) == -1) {
        perror("msync MAP_SHARED");
        (void) munmap(mapping, sizeof(initial) - 1);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (munmap(mapping, sizeof(initial) - 1) == -1) {
        perror("munmap MAP_SHARED");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (pread(fd, verification, sizeof(initial) - 1, 0) !=
        (ssize_t) (sizeof(initial) - 1)) {
        perror("pread shared mapping result");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1) {
        perror("close " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("mapping=%.*s file=%.*s msync=yes\n",
           (int) (sizeof(replacement) - 1), mapped_copy,
           (int) (sizeof(replacement) - 1), verification);
    return memcmp(mapped_copy, verification, sizeof(initial) - 1) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
