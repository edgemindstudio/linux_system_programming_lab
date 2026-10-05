/*
 * Experiment 09.B - Observe demand paging through minor page faults
 *
 * Purpose:
 *   Reserve anonymous virtual memory, then touch one byte per page and compare
 *   the process's page-fault counters before and after those first accesses.
 *
 * Linux behavior:
 *   mmap() can establish an address range without immediately supplying a
 *   private physical page for every virtual page. First writes commonly cause
 *   minor faults that the kernel resolves without storage I/O.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

enum { PAGE_COUNT = 128 };

int main(void)
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t length;
    unsigned char *mapping;
    struct rusage before;
    struct rusage after;
    long minor_delta;
    long major_delta;

    if (page_size <= 0) {
        fputs("sysconf(_SC_PAGESIZE) returned an invalid size\n", stderr);
        return EXIT_FAILURE;
    }
    length = (size_t)page_size * PAGE_COUNT;
    mapping = mmap(NULL,
                   length,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1,
                   0);
    if (mapping == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

#ifdef MADV_NOHUGEPAGE
    (void)madvise(mapping, length, MADV_NOHUGEPAGE);
#endif

    if (getrusage(RUSAGE_SELF, &before) == -1) {
        perror("getrusage before");
        (void)munmap(mapping, length);
        return EXIT_FAILURE;
    }
    for (size_t page = 0U; page < PAGE_COUNT; ++page) {
        mapping[page * (size_t)page_size] = (unsigned char)page;
    }
    if (getrusage(RUSAGE_SELF, &after) == -1) {
        perror("getrusage after");
        (void)munmap(mapping, length);
        return EXIT_FAILURE;
    }

    minor_delta = after.ru_minflt - before.ru_minflt;
    major_delta = after.ru_majflt - before.ru_majflt;
    if (munmap(mapping, length) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }

    printf("pages_touched=%d minor_fault_delta=%ld major_fault_delta=%ld\n",
           PAGE_COUNT,
           minor_delta,
           major_delta);
    printf("minor_faults_increased=%s major_fault_count_not_assumed=yes\n",
           minor_delta > 0 ? "yes" : "no");

    return minor_delta > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
