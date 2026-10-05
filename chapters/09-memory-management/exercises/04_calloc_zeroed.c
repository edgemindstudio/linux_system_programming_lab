/*
 * Exercise 09.04 - Obtain zero-filled array storage with calloc()
 *
 * Purpose:
 *   Contrast calloc() with malloc() by verifying the initial contents of an
 *   integer array before assigning application values.
 *
 * Linux behavior:
 *   calloc() requests one allocation for count multiplied by element size and
 *   initializes its bytes to zero. On Linux's integer representation, each
 *   resulting int compares equal to zero.
 */

#include <stdio.h>
#include <stdlib.h>

enum { ELEMENT_COUNT = 8 };

int main(void)
{
    int *values = calloc(ELEMENT_COUNT, sizeof(*values));
    int all_zero = 1;
    int sum = 0;

    if (values == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    for (int index = 0; index < ELEMENT_COUNT; ++index) {
        if (values[index] != 0) {
            all_zero = 0;
        }
        values[index] = index;
        sum += values[index];
    }

    printf("elements=%d initially_zero=%s assigned_sum=%d\n",
           ELEMENT_COUNT,
           all_zero ? "yes" : "no",
           sum);
    printf("count_and_element_size_supplied=yes allocation_released=yes\n");

    free(values);
    return all_zero && sum == 28 ? EXIT_SUCCESS : EXIT_FAILURE;
}
