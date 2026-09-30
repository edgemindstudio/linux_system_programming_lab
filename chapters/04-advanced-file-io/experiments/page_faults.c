/*
 * Experiment - Demand paging while walking a mapping
 *
 * Question:
 *   Does mmap() immediately populate every mapped page?
 *
 * Method:
 *   Record process fault counters, touch one byte per page, and record them
 *   again. Exact deltas vary because cache state, readahead, and the kernel's
 *   fault-around policy vary; the measurement is observational.
 */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/04-advanced-file-io/data/page_faults.bin"

enum { MAPPING_MEBIBYTES = 4 };

int main(void)
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t length = (size_t) MAPPING_MEBIBYTES * 1024U * 1024U;
    int fd;
    const unsigned char *mapping;
    struct rusage before;
    struct rusage after;
    size_t offset;
    size_t pages = 0;
    unsigned long checksum = 0;
    int release_failed = 0;

    if (page_size <= 0) {
        perror("sysconf _SC_PAGESIZE");
        return EXIT_FAILURE;
    }

    fd = open(OUTPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }
    if (ftruncate(fd, (off_t) length) == -1) {
        perror("ftruncate page-fault file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    mapping = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED) {
        perror("mmap page-fault file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (getrusage(RUSAGE_SELF, &before) == -1) {
        perror("getrusage before mapping walk");
        (void) munmap((void *) mapping, length);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    for (offset = 0; offset < length; offset += (size_t) page_size) {
        checksum += mapping[offset];
        ++pages;
    }

    if (getrusage(RUSAGE_SELF, &after) == -1) {
        perror("getrusage after mapping walk");
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
        perror("release page-fault resources");
        return EXIT_FAILURE;
    }

    printf("pages_touched=%zu minor_fault_delta=%ld major_fault_delta=%ld "
           "checksum=%lu\n",
           pages,
           after.ru_minflt - before.ru_minflt,
           after.ru_majflt - before.ru_majflt,
           checksum);
    return EXIT_SUCCESS;
}
