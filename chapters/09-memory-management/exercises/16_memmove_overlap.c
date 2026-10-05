/*
 * Exercise 09.16 - Move overlapping bytes safely
 *
 * Purpose:
 *   Shift part of a string within the same array using memmove().
 *
 * Linux behavior:
 *   memcpy() requires nonoverlapping source and destination ranges. memmove()
 *   behaves as though it first copied through temporary storage, making this
 *   deliberate overlap well-defined.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char text[7] = "abcdef";

    memmove(text + 2, text, 4U);

    printf("result=%s expected=ababcd\n", text);
    printf("overlap_handled=%s memmove_required=yes\n",
           strcmp(text, "ababcd") == 0 ? "yes" : "no");

    return strcmp(text, "ababcd") == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
