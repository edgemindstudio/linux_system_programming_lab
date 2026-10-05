/*
 * Exercise 09.07 - Request explicitly aligned dynamic memory
 *
 * Purpose:
 *   Allocate storage aligned to a 64-byte boundary and verify that alignment
 *   without assuming a particular numeric address.
 *
 * Linux behavior:
 *   malloc() already satisfies fundamental type alignment. posix_memalign()
 *   supports stronger power-of-two alignment needed by some SIMD, I/O, and
 *   cache-aware code. Unlike errno-style calls, it returns an error number.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const size_t alignment = 64U;
    const size_t size = 256U;
    void *storage = NULL;
    int result = posix_memalign(&storage, alignment, size);
    int aligned;

    if (result != 0) {
        fprintf(stderr, "posix_memalign: %s\n", strerror(result));
        return EXIT_FAILURE;
    }

    aligned = (uintptr_t)storage % alignment == 0U;
    memset(storage, 0x5a, size);

    printf("alignment=%zu size=%zu aligned=%s\n",
           alignment,
           size,
           aligned ? "yes" : "no");
    printf("power_of_two_alignment=yes ordinary_free_used=yes\n");

    free(storage);
    return aligned ? EXIT_SUCCESS : EXIT_FAILURE;
}
