/*
 * Exercise 09.03 - Allocate and initialize an array with malloc()
 *
 * Purpose:
 *   Practice the complete heap-allocation lifecycle: compute a byte count,
 *   check allocation failure, initialize every element, use the data, and
 *   release ownership exactly once.
 *
 * Linux behavior:
 *   malloc() returns suitably aligned storage but does not initialize it.
 *   Reading an element before this program writes it would be undefined C
 *   behavior, so every element is assigned before the sum is computed.
 */

#include <stdio.h>
#include <stdlib.h>

enum { ELEMENT_COUNT = 8 };

int main(void)
{
    int *values = malloc((size_t)ELEMENT_COUNT * sizeof(*values));
    int sum = 0;

    if (values == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    for (int index = 0; index < ELEMENT_COUNT; ++index) {
        values[index] = index + 1;
        sum += values[index];
    }

    printf("elements=%d bytes=%zu sum=%d\n",
           ELEMENT_COUNT,
           (size_t)ELEMENT_COUNT * sizeof(*values),
           sum);
    printf("every_element_initialized=yes ownership_released=yes sum_correct=%s\n",
           sum == 36 ? "yes" : "no");

    free(values);
    return sum == 36 ? EXIT_SUCCESS : EXIT_FAILURE;
}
