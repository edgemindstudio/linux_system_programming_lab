/*
 * Experiment 09.E - Discard private anonymous pages with MADV_DONTNEED
 *
 * Purpose:
 *   Fill a private anonymous page, tell Linux its current contents are not
 *   needed, and observe the zero-filled contents supplied on later access.
 *
 * Linux behavior:
 *   For a private anonymous mapping on Linux, MADV_DONTNEED discards the
 *   affected pages. A later read observes the mapping's zero-fill semantics.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
    long page_size = sysconf(_SC_PAGESIZE);
    unsigned char *page;
    int zero_after_advice = 1;

    if (page_size <= 0) {
        fputs("sysconf(_SC_PAGESIZE) returned an invalid size\n", stderr);
        return EXIT_FAILURE;
    }
    page = mmap(NULL,
                (size_t)page_size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1,
                0);
    if (page == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    memset(page, 0xa5, (size_t)page_size);
    if (madvise(page, (size_t)page_size, MADV_DONTNEED) == -1) {
        perror("madvise MADV_DONTNEED");
        (void)munmap(page, (size_t)page_size);
        return EXIT_FAILURE;
    }
    for (long index = 0; index < page_size; ++index) {
        if (page[index] != 0U) {
            zero_after_advice = 0;
            break;
        }
    }

    if (munmap(page, (size_t)page_size) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }
    printf("page_bytes=%ld zero_after_dontneed=%s\n",
           page_size,
           zero_after_advice != 0 ? "yes" : "no");
    printf("linux_private_anonymous_semantics_observed=%s\n",
           zero_after_advice != 0 ? "yes" : "no");

    return zero_after_advice != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
