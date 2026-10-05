/*
 * Exercise 09.05 - Grow an allocation with realloc()
 *
 * Purpose:
 *   Enlarge a four-element array to eight elements while preserving the old
 *   elements and safely handling a possible allocation failure.
 *
 * Linux behavior:
 *   realloc() may extend storage in place or move it. On success, the prefix
 *   up to the smaller old/new size is preserved. A temporary pointer prevents
 *   the original allocation from being lost when realloc() fails.
 */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const size_t old_count = 4U;
    const size_t new_count = 8U;
    int *values = malloc(old_count * sizeof(*values));
    int *resized;
    int prefix_preserved = 1;
    int sum = 0;

    if (values == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    for (size_t index = 0U; index < old_count; ++index) {
        values[index] = (int)index + 1;
    }

    resized = realloc(values, new_count * sizeof(*values));
    if (resized == NULL) {
        perror("realloc");
        free(values);
        return EXIT_FAILURE;
    }
    values = resized;

    for (size_t index = 0U; index < old_count; ++index) {
        if (values[index] != (int)index + 1) {
            prefix_preserved = 0;
        }
    }
    for (size_t index = old_count; index < new_count; ++index) {
        values[index] = (int)index + 1;
    }
    for (size_t index = 0U; index < new_count; ++index) {
        sum += values[index];
    }

    printf("old_count=%zu new_count=%zu prefix_preserved=%s sum=%d\n",
           old_count,
           new_count,
           prefix_preserved ? "yes" : "no",
           sum);
    printf("temporary_pointer_used=yes new_tail_initialized=yes\n");

    free(values);
    return prefix_preserved && sum == 36 ? EXIT_SUCCESS : EXIT_FAILURE;
}
