/*
 * Exercise 06.18 - Lock one resident page
 *
 * Purpose:
 *   Demonstrate the smallest practical memory-locking lifecycle: allocate an
 *   aligned page, fault it in, lock it, unlock it, and free it.
 *
 * Linux behavior:
 *   mlock() prevents the selected virtual-memory pages from being paged out.
 *   The operation is limited by RLIMIT_MEMLOCK and capabilities. A denied
 *   lock is reported as an environmental result rather than a lab failure.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
    void *page = NULL;
    long page_size = sysconf(_SC_PAGESIZE);
    int lock_result;
    int saved_errno;

    if (page_size <= 0) {
        fputs("invalid page size\n", stderr);
        return EXIT_FAILURE;
    }
    if (posix_memalign(&page, (size_t)page_size, (size_t)page_size) != 0) {
        fputs("posix_memalign failed\n", stderr);
        return EXIT_FAILURE;
    }

    memset(page, 0x5A, (size_t)page_size);
    errno = 0;
    lock_result = mlock(page, (size_t)page_size);
    saved_errno = errno;

    if (lock_result == -1) {
        printf("page_size=%ld mlock_supported=no errno=%d\n",
               page_size, saved_errno);
        puts("page_prefaulted=yes lock_released=not-needed");
        free(page);
        return EXIT_SUCCESS;
    }

    if (munlock(page, (size_t)page_size) == -1) {
        perror("munlock");
        free(page);
        return EXIT_FAILURE;
    }

    printf("page_size=%ld mlock_supported=yes errno=0\n", page_size);
    puts("page_prefaulted=yes lock_released=yes");
    free(page);
    return EXIT_SUCCESS;
}
