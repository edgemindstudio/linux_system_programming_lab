/*
 * Exercise 09.17 - Search a bounded byte region
 *
 * Purpose:
 *   Find one byte with memchr() without treating the buffer as a C string.
 *
 * Linux behavior:
 *   memchr() searches exactly the supplied byte count and may search buffers
 *   containing zeros. It returns a pointer into the original object or NULL.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const unsigned char bytes[] = {0x10U, 0x00U, 0x20U, 0x7fU, 0x30U};
    const unsigned char *found = memchr(bytes, 0x7f, sizeof(bytes));
    const unsigned char *missing = memchr(bytes, 0xff, sizeof(bytes));
    ptrdiff_t index = found == NULL ? -1 : found - bytes;

    printf("target_found=%s index=%td missing_found=%s\n",
           found != NULL ? "yes" : "no",
           index,
           missing != NULL ? "yes" : "no");
    printf("embedded_zero_safe=yes bounded_search=yes\n");

    return found != NULL && index == 3 && missing == NULL
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
