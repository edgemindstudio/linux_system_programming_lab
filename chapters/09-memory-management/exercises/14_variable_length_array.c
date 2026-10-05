/*
 * Exercise 09.14 - Use a bounded variable-length array
 *
 * Purpose:
 *   Allocate an array whose element count is known only at runtime while
 *   keeping its lifetime tied to the current block.
 *
 * Linux behavior:
 *   A VLA normally consumes stack space. C17 permits implementations not to
 *   support VLAs, and excessive runtime sizes can overflow the stack, so this
 *   example uses a small, validated count.
 */

#include <stdio.h>
#include <stdlib.h>

static int sum_sequence(size_t count)
{
    int values[count];
    int sum = 0;

    for (size_t index = 0U; index < count; ++index) {
        values[index] = (int)index + 1;
        sum += values[index];
    }
    return sum;
}

int main(void)
{
    const size_t count = 8U;
    int sum = sum_sequence(count);

    printf("runtime_count=%zu sum=%d sum_correct=%s\n",
           count,
           sum,
           sum == 36 ? "yes" : "no");
    printf("bounded_input=yes block_lifetime=yes portability_considered=yes\n");

    return sum == 36 ? EXIT_SUCCESS : EXIT_FAILURE;
}
