/*
 * Exercise 04.07 - Read a regular file through mmap()
 *
 * Purpose:
 *   Map a file into the process address space and access its bytes like memory.
 *
 * Linux behavior:
 *   mmap() adds its own reference to the backing file. After mmap() succeeds,
 *   the original descriptor can be closed while the mapping remains valid.
 *   munmap() ends that relationship and invalidates the mapped addresses.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define INPUT_PATH "build/chapters/04-advanced-file-io/data/mmap_read.txt"

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
    static const char text[] = "mapped bytes remain available";
    struct stat info;
    int fd = open(INPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    const char *mapping;

    if (fd == -1) {
        perror("open " INPUT_PATH);
        return EXIT_FAILURE;
    }

    if (write_all(fd, text, sizeof(text) - 1) == -1 ||
        fstat(fd, &info) == -1) {
        perror("initialize mmap input");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    mapping = mmap(NULL, (size_t) info.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED) {
        perror("mmap");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1) {
        perror("close descriptor after mmap");
        (void) munmap((void *) mapping, (size_t) info.st_size);
        return EXIT_FAILURE;
    }

    printf("descriptor_closed=yes mapped_text=%.*s\n",
           (int) info.st_size,
           mapping);

    if (munmap((void *) mapping, (size_t) info.st_size) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
