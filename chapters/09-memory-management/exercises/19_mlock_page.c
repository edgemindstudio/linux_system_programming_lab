/*
 * Exercise 09.19 - Attempt to lock one mapped page in memory
 *
 * Purpose:
 *   Demonstrate mlock() and munlock() while respecting the process's resource
 *   limit and the possibility that the execution environment denies locking.
 *
 * Linux behavior:
 *   Locking keeps selected pages from being paged out, but it is constrained
 *   by RLIMIT_MEMLOCK and privilege. A policy, resource, or temporary failure
 *   is reported as an environmental boundary rather than hidden or bypassed.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

int main(void)
{
    long page_size_long = sysconf(_SC_PAGESIZE);
    size_t page_size;
    unsigned char *mapping;
    struct rlimit limit;
    int lock_result;
    int lock_errno;

    if (page_size_long <= 0) {
        perror("sysconf _SC_PAGESIZE");
        return EXIT_FAILURE;
    }
    if (getrlimit(RLIMIT_MEMLOCK, &limit) == -1) {
        perror("getrlimit RLIMIT_MEMLOCK");
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
    mapping[0] = 0x2aU;

    errno = 0;
    lock_result = mlock(mapping, page_size);
    lock_errno = errno;
    if (lock_result == 0) {
        if (munlock(mapping, page_size) == -1) {
            perror("munlock");
            (void)munmap(mapping, page_size);
            return EXIT_FAILURE;
        }
        printf("mlock_supported=yes page_locked=yes page_unlocked=yes\n");
    } else if (lock_errno == EPERM || lock_errno == ENOMEM ||
               lock_errno == EAGAIN) {
        printf("mlock_supported=no constraint_errno=%d\n", lock_errno);
        printf("environmental_boundary=yes\n");
    } else {
        errno = lock_errno;
        perror("mlock");
        (void)munmap(mapping, page_size);
        return EXIT_FAILURE;
    }

    printf("soft_limit_infinite=%s soft_limit_bytes=%llu page_bytes=%zu\n",
           limit.rlim_cur == RLIM_INFINITY ? "yes" : "no",
           limit.rlim_cur == RLIM_INFINITY
               ? 0ULL
               : (unsigned long long)limit.rlim_cur,
           page_size);
    printf("secret_data_not_used=yes\n");
    if (munmap(mapping, page_size) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
