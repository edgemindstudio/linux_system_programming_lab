/*
 * Exercise 09.13 - Make a small scope-bound allocation with alloca()
 *
 * Purpose:
 *   Contrast stack-based dynamic storage with heap storage.
 *
 * Linux behavior:
 *   alloca() advances stack storage for the current function invocation. It
 *   requires no free(), but large or attacker-controlled sizes can overflow
 *   the finite thread stack and cannot be reported through a normal failure
 *   return. This example intentionally requests only 64 bytes.
 */

#define _GNU_SOURCE

#include <alloca.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const size_t size = 64U;
    unsigned char *storage = alloca(size);
    int contents_ok = 1;

    memset(storage, 0x2a, size);
    for (size_t index = 0U; index < size; ++index) {
        if (storage[index] != 0x2aU) {
            contents_ok = 0;
            break;
        }
    }

    printf("stack_bytes=%zu contents_ok=%s\n",
           size,
           contents_ok ? "yes" : "no");
    printf("scope_bound=yes explicit_free_required=no large_sizes_avoided=yes\n");

    return contents_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
