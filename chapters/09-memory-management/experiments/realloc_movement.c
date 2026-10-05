/*
 * Experiment 09.D - Allow realloc() to move an allocation
 *
 * Purpose:
 *   Grow a small allocation substantially and verify its bytes survive even
 *   though the allocation is permitted to move to a different address.
 *
 * Linux behavior:
 *   After a successful realloc(), the old pointer is invalid even when the
 *   numeric address happens to be unchanged. This program saves only the old
 *   address value for reporting and uses the returned pointer thereafter.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { LARGE_SIZE = 8 * 1024 * 1024 };

int main(void)
{
    const char marker[] = "realloc-preserved-this";
    unsigned char *block = malloc(64U);
    uintptr_t old_address;
    unsigned char *grown;
    int preserved;
    int moved;

    if (block == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    memcpy(block, marker, sizeof(marker));
    old_address = (uintptr_t)block;

    grown = realloc(block, LARGE_SIZE);
    if (grown == NULL) {
        perror("realloc");
        free(block);
        return EXIT_FAILURE;
    }
    preserved = memcmp(grown, marker, sizeof(marker)) == 0;
    moved = old_address != (uintptr_t)grown;
    grown[LARGE_SIZE - 1U] = 0x5aU;

    printf("grown_bytes=%d data_preserved=%s allocation_moved=%s\n",
           LARGE_SIZE,
           preserved != 0 ? "yes" : "no",
           moved != 0 ? "yes" : "no");
    printf("movement_is_allowed=yes old_pointer_not_reused=yes\n");

    free(grown);
    return preserved != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
