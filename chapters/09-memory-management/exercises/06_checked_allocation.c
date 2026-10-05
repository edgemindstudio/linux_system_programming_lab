/*
 * Exercise 09.06 - Reject an overflowing allocation-size calculation
 *
 * Purpose:
 *   Prevent count * element_size from wrapping before memory is requested.
 *
 * Linux behavior:
 *   size_t arithmetic is unsigned and wraps modulo its range. If a wrapped
 *   byte count reaches malloc(), the allocation can be much smaller than the
 *   caller expects. The explicit division check rejects that condition first.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void *checked_calloc(size_t count, size_t element_size)
{
    if (count != 0U && element_size > SIZE_MAX / count) {
        errno = ENOMEM;
        return NULL;
    }
    return calloc(count, element_size);
}

int main(void)
{
    void *rejected;
    int rejected_errno;
    int *safe;

    errno = 0;
    rejected = checked_calloc(SIZE_MAX, 2U);
    rejected_errno = errno;
    safe = checked_calloc(16U, sizeof(*safe));

    if (safe == NULL) {
        perror("checked_calloc safe request");
        return EXIT_FAILURE;
    }

    printf("overflow_rejected=%s errno_is_enomem=%s safe_allocation=yes\n",
           rejected == NULL ? "yes" : "no",
           rejected_errno == ENOMEM ? "yes" : "no");
    printf("multiplication_checked_before_allocator=yes\n");

    free(safe);
    return rejected == NULL && rejected_errno == ENOMEM
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
