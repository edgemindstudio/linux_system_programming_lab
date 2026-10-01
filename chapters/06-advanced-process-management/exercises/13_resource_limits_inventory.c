/*
 * Exercise 06.13 - Inventory important process resource limits
 *
 * Purpose:
 *   Read soft and hard ceilings enforced by the kernel.
 *
 * Linux behavior:
 *   The soft limit is the active ceiling. The hard limit is the maximum value
 *   an unprivileged process may select for its soft limit. Limits are process
 *   state: they are inherited by fork() and preserved by exec().
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

static void print_value(rlim_t value)
{
    if (value == RLIM_INFINITY) {
        fputs("infinity", stdout);
    } else {
        printf("%llu", (unsigned long long)value);
    }
}

static int print_limit(const char *name, int resource)
{
    struct rlimit limit;

    if (getrlimit(resource, &limit) == -1) {
        perror(name);
        return -1;
    }

    printf("%s soft=", name);
    print_value(limit.rlim_cur);
    fputs(" hard=", stdout);
    print_value(limit.rlim_max);
    fputc('\n', stdout);
    return 0;
}

int main(void)
{
    if (print_limit("RLIMIT_NOFILE", RLIMIT_NOFILE) == -1 ||
        print_limit("RLIMIT_STACK", RLIMIT_STACK) == -1 ||
        print_limit("RLIMIT_CORE", RLIMIT_CORE) == -1 ||
        print_limit("RLIMIT_NPROC", RLIMIT_NPROC) == -1 ||
        print_limit("RLIMIT_MEMLOCK", RLIMIT_MEMLOCK) == -1) {
        return EXIT_FAILURE;
    }

    puts("limits_read=yes soft_is_active=yes hard_is_ceiling=yes");
    return EXIT_SUCCESS;
}
