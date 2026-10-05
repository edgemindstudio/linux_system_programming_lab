/*
 * Exercise 09.12 - Offer freed heap memory back to the system
 *
 * Purpose:
 *   Call glibc's malloc_trim() after releasing a moderately large allocation.
 *
 * Linux behavior:
 *   free() returns storage to the allocator, not necessarily to the kernel.
 *   malloc_trim() asks glibc to release suitable free heap pages. A return of
 *   zero is not an error; allocator layout may simply leave nothing trimmable.
 */

#define _GNU_SOURCE

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const size_t allocation_size = 4U * 1024U * 1024U;
    unsigned char *storage = malloc(allocation_size);
    int trim_result;

    if (storage == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    memset(storage, 0x55, allocation_size);
    free(storage);

    trim_result = malloc_trim(0U);
    printf("allocation_bytes=%zu malloc_trim_result=%d\n",
           allocation_size,
           trim_result);
    printf("trim_called=yes zero_is_not_failure=yes allocator_decides_release=yes\n");

    return EXIT_SUCCESS;
}
