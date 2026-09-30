/*
 * Exercise 04.13 - Describe a mapping's access pattern with madvise()
 *
 * Purpose:
 *   Tell the kernel that a file mapping will be consumed sequentially and soon.
 *
 * Linux behavior:
 *   madvise() communicates a performance hint, not a correctness requirement.
 *   The kernel may adjust readahead or cache policy, but successful advice does
 *   not promise a specific timing or number of physical I/O operations.
 */

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/04-advanced-file-io/data/madvise_mapping.bin"

int main(void)
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t length;
    int fd;
    int release_failed = 0;
    const unsigned char *mapping;
    unsigned int checksum;

    if (page_size <= 0) {
        perror("sysconf _SC_PAGESIZE");
        return EXIT_FAILURE;
    }

    length = (size_t) page_size * 2;
    fd = open(OUTPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (ftruncate(fd, (off_t) length) == -1 ||
        pwrite(fd, "A", 1, 0) != 1 ||
        pwrite(fd, "Z", 1, (off_t) length - 1) != 1) {
        perror("prepare advised mapping");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    mapping = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED) {
        perror("mmap advised file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (madvise((void *) mapping, length, MADV_SEQUENTIAL) == -1 ||
        madvise((void *) mapping, length, MADV_WILLNEED) == -1) {
        perror("madvise");
        (void) munmap((void *) mapping, length);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    checksum = (unsigned int) mapping[0] +
               (unsigned int) mapping[length - 1];

    if (munmap((void *) mapping, length) == -1) {
        release_failed = 1;
    }
    if (close(fd) == -1) {
        release_failed = 1;
    }
    if (release_failed != 0) {
        perror("release advised mapping resources");
        return EXIT_FAILURE;
    }

    printf("advice=sequential+willneed bytes=%zu checksum=%u accepted=yes\n",
           length, checksum);
    return EXIT_SUCCESS;
}
