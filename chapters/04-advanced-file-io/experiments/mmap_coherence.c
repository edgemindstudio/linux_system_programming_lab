/*
 * Experiment - MAP_SHARED visibility versus storage synchronization
 *
 * Question:
 *   Must msync() run before descriptor I/O can observe a MAP_SHARED change?
 *
 * Interpretation:
 *   Linux unifies file mappings and descriptor I/O through the page cache, so
 *   pread() sees the shared change immediately. msync(MS_SYNC) addresses the
 *   separate synchronization boundary; it is not required merely for coherence.
 */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/04-advanced-file-io/data/mmap_coherence.txt"

int main(void)
{
    char observed[5] = {0};
    int fd = open(OUTPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    int release_failed = 0;
    char *mapping;

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }
    if (pwrite(fd, "cold", 4, 0) != 4) {
        perror("initialize coherence file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    mapping = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED) {
        perror("mmap coherence file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    memcpy(mapping, "warm", 4);
    if (pread(fd, observed, 4, 0) != 4) {
        perror("pread before msync");
        (void) munmap(mapping, 4);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (msync(mapping, 4, MS_SYNC) == -1) {
        perror("msync coherence file");
        (void) munmap(mapping, 4);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (munmap(mapping, 4) == -1) {
        release_failed = 1;
    }
    if (close(fd) == -1) {
        release_failed = 1;
    }
    if (release_failed != 0) {
        perror("release coherence resources");
        return EXIT_FAILURE;
    }

    printf("descriptor_before_msync=%s coherent=%s synchronization=done\n",
           observed,
           strcmp(observed, "warm") == 0 ? "yes" : "no");
    return strcmp(observed, "warm") == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
