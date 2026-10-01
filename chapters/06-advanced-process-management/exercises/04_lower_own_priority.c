/*
 * Exercise 06.04 - Lower this process's scheduling priority
 *
 * Purpose:
 *   Demonstrate the direction of the Unix nice scale safely.
 *
 * Linux behavior:
 *   A numerically larger nice value means a less favored normal process.
 *   Unprivileged processes may normally increase their own nice value, but
 *   decreasing it again requires privilege or a suitable RLIMIT_NICE value.
 *   This short-lived program exits with the changed value, so the shell is
 *   unaffected.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

static int read_nice(void)
{
    int value;

    errno = 0;
    value = getpriority(PRIO_PROCESS, 0);
    if (value == -1 && errno != 0) {
        return 1000;
    }
    return value;
}

int main(void)
{
    int before = read_nice();
    int requested;
    int after;

    if (before == 1000) {
        perror("getpriority before");
        return EXIT_FAILURE;
    }

    requested = before < 19 ? before + 1 : before;
    if (setpriority(PRIO_PROCESS, 0, requested) == -1) {
        perror("setpriority");
        return EXIT_FAILURE;
    }

    after = read_nice();
    if (after == 1000) {
        perror("getpriority after");
        return EXIT_FAILURE;
    }

    printf("before=%d requested=%d after=%d\n", before, requested, after);
    printf("priority_not_raised=%s shell_unchanged=yes\n",
           after >= before ? "yes" : "no");

    return EXIT_SUCCESS;
}
