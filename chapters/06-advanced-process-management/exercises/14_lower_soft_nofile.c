/*
 * Exercise 06.14 - Temporarily lower and restore a soft limit
 *
 * Purpose:
 *   Change RLIMIT_NOFILE while preserving its hard ceiling.
 *
 * Linux behavior:
 *   An unprivileged process may lower a soft limit and later raise it back up
 *   to the unchanged hard limit. Lowering the hard limit would be effectively
 *   irreversible for that process and is deliberately avoided here.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

int main(void)
{
    struct rlimit original;
    struct rlimit temporary;
    struct rlimit observed;
    struct rlimit restored;

    if (getrlimit(RLIMIT_NOFILE, &original) == -1) {
        perror("getrlimit original");
        return EXIT_FAILURE;
    }

    temporary = original;
    if (temporary.rlim_cur == RLIM_INFINITY || temporary.rlim_cur > 64U) {
        temporary.rlim_cur = 64U;
    }

    if (setrlimit(RLIMIT_NOFILE, &temporary) == -1) {
        perror("setrlimit temporary");
        return EXIT_FAILURE;
    }
    if (getrlimit(RLIMIT_NOFILE, &observed) == -1) {
        perror("getrlimit observed");
        return EXIT_FAILURE;
    }
    if (setrlimit(RLIMIT_NOFILE, &original) == -1) {
        perror("setrlimit restore");
        return EXIT_FAILURE;
    }
    if (getrlimit(RLIMIT_NOFILE, &restored) == -1) {
        perror("getrlimit restored");
        return EXIT_FAILURE;
    }

    printf("temporary_soft=%llu hard_preserved=%s\n",
           (unsigned long long)observed.rlim_cur,
           observed.rlim_max == original.rlim_max ? "yes" : "no");
    printf("temporary_applied=%s original_restored=%s\n",
           observed.rlim_cur == temporary.rlim_cur ? "yes" : "no",
           restored.rlim_cur == original.rlim_cur &&
                   restored.rlim_max == original.rlim_max
               ? "yes" : "no");

    return EXIT_SUCCESS;
}
