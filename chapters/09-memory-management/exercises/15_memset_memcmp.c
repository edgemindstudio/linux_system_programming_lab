/*
 * Exercise 09.15 - Set and compare raw memory
 *
 * Purpose:
 *   Use memset() to establish byte patterns and memcmp() to compare a fixed
 *   number of bytes.
 *
 * Linux behavior:
 *   These C-library operations treat memory as unsigned bytes. memcmp()
 *   reports lexical ordering through a value less than, equal to, or greater
 *   than zero; portable code must not expect exactly -1 or 1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    unsigned char first[16];
    unsigned char second[16];
    int equal_before;
    int different_after;

    memset(first, 0x5a, sizeof(first));
    memset(second, 0x5a, sizeof(second));
    equal_before = memcmp(first, second, sizeof(first)) == 0;

    second[7] = 0x5b;
    different_after = memcmp(first, second, sizeof(first)) != 0;

    printf("equal_before=%s different_after=%s compared_bytes=%zu\n",
           equal_before ? "yes" : "no",
           different_after ? "yes" : "no",
           sizeof(first));
    printf("comparison_sign_contract_used=yes raw_bytes_compared=yes\n");

    return equal_before && different_after ? EXIT_SUCCESS : EXIT_FAILURE;
}
