/*
 * Exercise 09.18 - Query whether an anonymous page is resident
 *
 * Purpose:
 *   Use mincore() before and after touching a page and interpret only the
 *   post-touch residency guarantee needed by this controlled experiment.
 *
 * Linux behavior:
 *   mmap() reserves virtual address space, while physical backing is often
 *   established on first access. Initial residency can vary with kernel
 *   policy, so the program reports it but does not require one initial value.
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
    unsigned char vector_before = 0U;
    unsigned char vector_after = 0U;
    unsigned char *mapping;
    int resident_before;
    int resident_after;

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
    if (mincore(mapping, page_size, &vector_before) == -1) {
        perror("mincore before");
        (void)munmap(mapping, page_size);
        return EXIT_FAILURE;
    }

    mapping[0] = 1U;
    if (mincore(mapping, page_size, &vector_after) == -1) {
        perror("mincore after");
        (void)munmap(mapping, page_size);
        return EXIT_FAILURE;
    }
    resident_before = (vector_before & 1U) != 0U;
    resident_after = (vector_after & 1U) != 0U;

    printf("resident_before=%s resident_after_touch=%s\n",
           resident_before ? "yes" : "no",
           resident_after ? "yes" : "no");
    printf("initial_state_not_assumed=yes page_granular_query=yes\n");

    if (munmap(mapping, page_size) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }
    return resident_after ? EXIT_SUCCESS : EXIT_FAILURE;
}
