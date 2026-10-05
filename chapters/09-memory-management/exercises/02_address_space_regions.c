/*
 * Exercise 09.02 - Distinguish process address-space regions
 *
 * Purpose:
 *   Place objects in initialized global storage, zero-initialized global
 *   storage, the heap, and the current thread's stack.
 *
 * Linux behavior:
 *   These objects occupy different logical regions in one virtual address
 *   space. Their exact addresses and ordering are deliberately not assumed;
 *   ASLR, the linker, the allocator, and the runtime may change them.
 */

#include <stdio.h>
#include <stdlib.h>

static int initialized_global = 17;
static int zero_initialized_global;

int main(void)
{
    int stack_value = 29;
    int *heap_value = malloc(sizeof(*heap_value));
    int all_distinct;

    if (heap_value == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    *heap_value = 41;

    all_distinct =
        (void *)&initialized_global != (void *)&zero_initialized_global &&
        (void *)&initialized_global != (void *)&stack_value &&
        (void *)&initialized_global != (void *)heap_value &&
        (void *)&zero_initialized_global != (void *)&stack_value &&
        (void *)&zero_initialized_global != (void *)heap_value &&
        (void *)&stack_value != (void *)heap_value;

    printf("initialized_global=%d zero_initialized_global=%d "
           "stack=%d heap=%d\n",
           initialized_global,
           zero_initialized_global,
           stack_value,
           *heap_value);
    printf("regions_distinct=%s address_order_not_assumed=yes "
           "one_virtual_address_space=yes\n",
           all_distinct ? "yes" : "no");

    free(heap_value);
    return all_distinct ? EXIT_SUCCESS : EXIT_FAILURE;
}
