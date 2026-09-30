/*
 * Exercise 04.10 - Use the runtime page size for an mmap() offset
 *
 * Purpose:
 *   Obtain the system page size and map the second page of a file.
 *
 * Linux behavior:
 *   A file-mapping offset must be a multiple of the system page size. Querying
 *   _SC_PAGESIZE at runtime avoids hard-coding an architecture-specific value.
 */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/04-advanced-file-io/data/aligned_mapping.bin"

int main(void)
{
    static const char marker[] = "PAGE2";
    long page_size = sysconf(_SC_PAGESIZE);
    int fd;
    int release_failed = 0;
    off_t offset;
    size_t length;
    const char *mapping;

    if (page_size <= 0) {
        perror("sysconf _SC_PAGESIZE");
        return EXIT_FAILURE;
    }

    length = (size_t) page_size;
    offset = (off_t) page_size;
    fd = open(OUTPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (ftruncate(fd, (off_t) (2 * page_size)) == -1 ||
        pwrite(fd, marker, sizeof(marker) - 1, offset) !=
            (ssize_t) (sizeof(marker) - 1)) {
        perror("prepare aligned mapping");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    mapping = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, offset);
    if (mapping == MAP_FAILED) {
        perror("mmap aligned offset");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (memcmp(mapping, marker, sizeof(marker) - 1) != 0) {
        fputs("mapped second-page marker did not match\n", stderr);
        (void) munmap((void *) mapping, length);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (munmap((void *) mapping, length) == -1) {
        release_failed = 1;
    }
    if (close(fd) == -1) {
        release_failed = 1;
    }
    if (release_failed != 0) {
        perror("release aligned mapping resources");
        return EXIT_FAILURE;
    }

    printf("page_size=%ld offset=%lld aligned=yes marker=%s\n",
           page_size, (long long) offset, marker);
    return EXIT_SUCCESS;
}
