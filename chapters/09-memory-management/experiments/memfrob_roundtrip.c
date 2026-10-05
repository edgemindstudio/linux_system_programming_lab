/*
 * Experiment 09.F - Apply GNU memfrob() twice
 *
 * Purpose:
 *   Demonstrate the GNU byte transformation mentioned in the chapter and
 *   make its security limitation explicit.
 *
 * Linux behavior:
 *   memfrob() XORs each byte with a fixed value. Applying it twice restores
 *   the original data, which makes it reversible obfuscation, not encryption.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char original[] = "memory-lab";
    char transformed[sizeof(original)];
    int changed;
    int restored;

    memcpy(transformed, original, sizeof(original));
    (void)memfrob(transformed, sizeof(transformed) - 1U);
    changed = memcmp(transformed, original, sizeof(original) - 1U) != 0;
    (void)memfrob(transformed, sizeof(transformed) - 1U);
    restored = memcmp(transformed, original, sizeof(original)) == 0;

    printf("changed_after_once=%s restored_after_twice=%s\n",
           changed != 0 ? "yes" : "no",
           restored != 0 ? "yes" : "no");
    printf("obfuscation_only=yes cryptography=no\n");

    return changed != 0 && restored != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
