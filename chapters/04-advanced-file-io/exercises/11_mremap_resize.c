/*
 * Exercise 04.11 - Grow an anonymous mapping with mremap()
 *
 * Purpose:
 *   Resize a one-page mapping while preserving its existing contents.
 *
 * Linux behavior:
 *   mremap() is Linux-specific. MREMAP_MAYMOVE permits the kernel to relocate
 *   the region, so callers must use the returned pointer and discard the old
 *   address after a successful call.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
    static const char message[] = "preserved-after-mremap";
    long page_size = sysconf(_SC_PAGESIZE);
    size_t old_size;
    size_t new_size;
    char *mapping;
    char *resized;

    if (page_size <= 0) {
        perror("sysconf _SC_PAGESIZE");
        return EXIT_FAILURE;
    }

    old_size = (size_t) page_size;
    new_size = old_size * 2;
    mapping = mmap(NULL,
                   old_size,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1,
                   0);
    if (mapping == MAP_FAILED) {
        perror("mmap anonymous region");
        return EXIT_FAILURE;
    }

    memcpy(mapping, message, sizeof(message));
    resized = mremap(mapping, old_size, new_size, MREMAP_MAYMOVE);
    if (resized == MAP_FAILED) {
        perror("mremap");
        (void) munmap(mapping, old_size);
        return EXIT_FAILURE;
    }

    resized[old_size] = 'X';
    if (strcmp(resized, message) != 0 || resized[old_size] != 'X') {
        fputs("mremap did not preserve or expand the region\n", stderr);
        (void) munmap(resized, new_size);
        return EXIT_FAILURE;
    }

    if (munmap(resized, new_size) == -1) {
        perror("munmap resized region");
        return EXIT_FAILURE;
    }

    printf("old_size=%zu new_size=%zu preserved=yes expanded=yes\n",
           old_size, new_size);
    return EXIT_SUCCESS;
}
