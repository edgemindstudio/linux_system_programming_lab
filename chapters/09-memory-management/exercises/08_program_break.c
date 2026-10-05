/*
 * Exercise 09.08 - Observe the process program break safely
 *
 * Purpose:
 *   Query the current end of the data segment without changing it.
 *
 * Linux behavior:
 *   brk() and sbrk() are historical low-level heap mechanisms. Modern code
 *   should normally use an allocator because libc coordinates the heap and
 *   may also obtain large allocations from mmap(). Directly moving the break
 *   behind the allocator's back can corrupt allocator state.
 */

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    void *current_break = sbrk(0);

    if (current_break == (void *)-1) {
        perror("sbrk query");
        return EXIT_FAILURE;
    }

    printf("program_break_query=yes address_nonnull=%s\n",
           current_break != NULL ? "yes" : "no");
    printf("legacy_interface=yes direct_break_change_avoided=yes "
           "allocator_should_manage_heap=yes\n");

    return EXIT_SUCCESS;
}
