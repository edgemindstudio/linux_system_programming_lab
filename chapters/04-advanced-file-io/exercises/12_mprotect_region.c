/*
 * Exercise 04.12 - Change a mapping from writable to read-only
 *
 * Purpose:
 *   Create data in a writable mapping, then tighten its page protection.
 *
 * Linux behavior:
 *   mprotect() changes permissions at page granularity. The program continues
 *   reading after PROT_READ is installed. A subsequent write would generate
 *   SIGSEGV, so this deterministic exercise does not intentionally perform it.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
    static const char message[] = "protected mapping";
    long page_size = sysconf(_SC_PAGESIZE);
    size_t length;
    char *mapping;

    if (page_size <= 0) {
        perror("sysconf _SC_PAGESIZE");
        return EXIT_FAILURE;
    }

    length = (size_t) page_size;
    mapping = mmap(NULL,
                   length,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1,
                   0);
    if (mapping == MAP_FAILED) {
        perror("mmap anonymous region");
        return EXIT_FAILURE;
    }

    memcpy(mapping, message, sizeof(message));
    if (mprotect(mapping, length, PROT_READ) == -1) {
        perror("mprotect PROT_READ");
        (void) munmap(mapping, length);
        return EXIT_FAILURE;
    }

    printf("text=%s protection=read-only page_granularity=yes\n", mapping);

    if (munmap(mapping, length) == -1) {
        perror("munmap protected region");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
