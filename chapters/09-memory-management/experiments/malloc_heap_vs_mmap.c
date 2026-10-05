/*
 * Experiment 09.C - Observe allocator strategy without treating it as a rule
 *
 * Purpose:
 *   Compare the process break around a small and a large allocation while
 *   recognizing that malloc() may use the heap, anonymous mappings, caches,
 *   or a mixture chosen by the C library.
 *
 * Linux behavior:
 *   malloc() promises suitably aligned storage, not a particular kernel
 *   mechanism. Thresholds and reuse behavior are implementation details and
 *   may change between runs or libc versions.
 */

#define _DEFAULT_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
    SMALL_SIZE = 1024,
    LARGE_SIZE = 16 * 1024 * 1024,
};

int main(void)
{
    void *break_before = sbrk(0);
    unsigned char *small;
    void *break_after_small;
    unsigned char *large;
    void *break_after_large;
    int small_changed;
    int large_changed;

    if (break_before == (void *)-1) {
        perror("sbrk before");
        return EXIT_FAILURE;
    }
    small = malloc(SMALL_SIZE);
    if (small == NULL) {
        perror("malloc small");
        return EXIT_FAILURE;
    }
    memset(small, 0x11, SMALL_SIZE);
    break_after_small = sbrk(0);
    if (break_after_small == (void *)-1) {
        perror("sbrk after small");
        free(small);
        return EXIT_FAILURE;
    }

    large = malloc(LARGE_SIZE);
    if (large == NULL) {
        perror("malloc large");
        free(small);
        return EXIT_FAILURE;
    }
    memset(large, 0x22, LARGE_SIZE);
    break_after_large = sbrk(0);
    if (break_after_large == (void *)-1) {
        perror("sbrk after large");
        free(large);
        free(small);
        return EXIT_FAILURE;
    }

    small_changed = (uintptr_t)break_before != (uintptr_t)break_after_small;
    large_changed =
        (uintptr_t)break_after_small != (uintptr_t)break_after_large;
    printf("small_bytes=%d large_bytes=%d break_after_small_changed=%s\n",
           SMALL_SIZE,
           LARGE_SIZE,
           small_changed != 0 ? "yes" : "no");
    printf("break_after_large_changed=%s allocations_succeeded=yes\n",
           large_changed != 0 ? "yes" : "no");
    printf("allocator_strategy_is_not_an_api_contract=yes\n");

    free(large);
    free(small);
    return EXIT_SUCCESS;
}
