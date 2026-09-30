/*
 * Exercise 04.09 - Observe copy-on-write with MAP_PRIVATE
 *
 * Purpose:
 *   Modify a private writable mapping and compare it with the backing file.
 *
 * Linux behavior:
 *   MAP_PRIVATE creates a copy-on-write view. The process sees its modified
 *   page, but the dirty private page is not written back to the file.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/04-advanced-file-io/data/mmap_private.txt"

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
    static const char initial[] = "original";
    static const char private_text[] = "private!";
    char memory_copy[sizeof(initial)] = {0};
    char file_copy[sizeof(initial)] = {0};
    int fd = open(OUTPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    int release_failed = 0;
    char *mapping;

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (write_all(fd, initial, sizeof(initial) - 1) == -1) {
        perror("initialize private mapping file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    mapping = mmap(NULL,
                   sizeof(initial) - 1,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE,
                   fd,
                   0);
    if (mapping == MAP_FAILED) {
        perror("mmap MAP_PRIVATE");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    memcpy(mapping, private_text, sizeof(private_text) - 1);
    memcpy(memory_copy, mapping, sizeof(initial) - 1);

    if (pread(fd, file_copy, sizeof(initial) - 1, 0) !=
        (ssize_t) (sizeof(initial) - 1)) {
        perror("pread private mapping backing file");
        (void) munmap(mapping, sizeof(initial) - 1);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (munmap(mapping, sizeof(initial) - 1) == -1) {
        release_failed = 1;
    }
    if (close(fd) == -1) {
        release_failed = 1;
    }
    if (release_failed != 0) {
        perror("release private mapping resources");
        return EXIT_FAILURE;
    }

    printf("memory=%s file=%s copy_on_write=yes\n", memory_copy, file_copy);
    return strcmp(memory_copy, private_text) == 0 &&
                   strcmp(file_copy, initial) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
