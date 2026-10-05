/*
 * Exercise 09.09 - Allocate zero-filled pages with anonymous mmap()
 *
 * Purpose:
 *   Request one page directly from the virtual-memory interface, verify its
 *   initial bytes, use the mapping, and return it with munmap().
 *
 * Linux behavior:
 *   MAP_ANONYMOUS creates a mapping with no file backing. New anonymous pages
 *   read as zero. Physical frames are commonly supplied lazily when pages are
 *   first accessed, not necessarily when mmap() returns.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
    long page_size_long = sysconf(_SC_PAGESIZE);
    size_t page_size;
    unsigned char *mapping;
    int initially_zero = 1;

    if (page_size_long <= 0) {
        perror("sysconf _SC_PAGESIZE");
        return EXIT_FAILURE;
    }
    page_size = (size_t)page_size_long;

    mapping = mmap(NULL,
                   page_size,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1,
                   0);
    if (mapping == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    for (size_t index = 0U; index < page_size; ++index) {
        if (mapping[index] != 0U) {
            initially_zero = 0;
            break;
        }
    }
    mapping[0] = 0x2aU;
    mapping[page_size - 1U] = 0x7fU;

    printf("page_bytes=%zu initially_zero=%s endpoints_writable=%s\n",
           page_size,
           initially_zero ? "yes" : "no",
           mapping[0] == 0x2aU && mapping[page_size - 1U] == 0x7fU
               ? "yes"
               : "no");
    printf("file_backing=no release_with_munmap=yes\n");

    if (munmap(mapping, page_size) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }
    return initially_zero ? EXIT_SUCCESS : EXIT_FAILURE;
}
