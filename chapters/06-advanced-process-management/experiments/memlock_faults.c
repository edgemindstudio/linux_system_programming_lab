/*
 * Experiment - Relate prefaulting, page faults, and memory locking
 *
 * Prediction:
 *   First writes to newly allocated pages should usually increase minor page
 *   faults. Locking already-touched pages should avoid demand-faulting those
 *   pages later, subject to RLIMIT_MEMLOCK.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

int main(void)
{
    const size_t page_count = 64U;
    long page_size = sysconf(_SC_PAGESIZE);
    unsigned char *memory = NULL;
    struct rusage before;
    struct rusage after_touch;
    long minor_delta;
    int lock_result;
    int lock_errno;

    if (page_size <= 0) {
        fputs("invalid page size\n", stderr);
        return EXIT_FAILURE;
    }
    if (posix_memalign((void **)&memory, (size_t)page_size,
                       page_count * (size_t)page_size) != 0) {
        fputs("posix_memalign failed\n", stderr);
        return EXIT_FAILURE;
    }
    if (getrusage(RUSAGE_SELF, &before) == -1) {
        perror("getrusage before");
        free(memory);
        return EXIT_FAILURE;
    }

    for (size_t page = 0U; page < page_count; ++page) {
        memory[page * (size_t)page_size] = (unsigned char)page;
    }

    if (getrusage(RUSAGE_SELF, &after_touch) == -1) {
        perror("getrusage after");
        free(memory);
        return EXIT_FAILURE;
    }
    minor_delta = after_touch.ru_minflt - before.ru_minflt;

    errno = 0;
    lock_result = mlock(memory, page_count * (size_t)page_size);
    lock_errno = errno;
    if (lock_result == 0 &&
        munlock(memory, page_count * (size_t)page_size) == -1) {
        perror("munlock");
        free(memory);
        return EXIT_FAILURE;
    }

    printf("pages_touched=%zu minor_fault_delta=%ld\n",
           page_count, minor_delta);
    printf("mlock_succeeded=%s errno=%d prefault_before_lock=yes\n",
           lock_result == 0 ? "yes" : "no", lock_errno);
    free(memory);
    return EXIT_SUCCESS;
}
