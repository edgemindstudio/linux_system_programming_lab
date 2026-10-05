/*
 * Exercise 09.11 - Inspect allocator-provided usable size
 *
 * Purpose:
 *   Compare a requested allocation size with glibc's internal usable size.
 *
 * Linux behavior:
 *   Allocators round requests into size classes and retain bookkeeping. The
 *   GNU malloc_usable_size() interface may report extra capacity, but portable
 *   application logic must continue treating only the requested bytes as its
 *   contract and must never depend on a particular amount of padding.
 */

#define _GNU_SOURCE

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const size_t requested = 37U;
    unsigned char *storage = malloc(requested);
    size_t usable;

    if (storage == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    usable = malloc_usable_size(storage);
    memset(storage, 0x3c, requested);

    printf("requested=%zu usable_at_least_requested=%s\n",
           requested,
           usable >= requested ? "yes" : "no");
    printf("only_requested_bytes_used=yes usable_size_is_allocator_specific=yes\n");

    free(storage);
    return usable >= requested ? EXIT_SUCCESS : EXIT_FAILURE;
}
