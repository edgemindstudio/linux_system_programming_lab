/*
 * Experiment — Collect child status and resource usage with wait4()
 *
 * Prediction:
 *   One wait4() call reaps the child and returns both its encoded status and a
 *   resource-usage snapshot accumulated for that child.
 *
 * Portability:
 *   wait4() is widely available on Linux and BSD systems but is not the core
 *   POSIX waiting interface. Portable code can combine waitpid() with broader
 *   getrusage() measurements when per-child wait4() data is unnecessary.
 */

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static long long microseconds(struct timeval value)
{
    return (long long)value.tv_sec * 1000000LL + (long long)value.tv_usec;
}

int main(void)
{
    pid_t child = fork();
    int status;
    struct rusage usage;

    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        volatile unsigned long accumulator = 0UL;
        for (unsigned long index = 0UL; index < 1000000UL; ++index) {
            accumulator += index % 17UL;
        }
        (void)accumulator;
        _exit(9);
    }

    if (wait4(child, &status, 0, &usage) == -1) {
        perror("wait4");
        return EXIT_FAILURE;
    }

    printf("exit_status=%d user_us=%lld system_us=%lld minor_faults=%ld\n",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1,
           microseconds(usage.ru_utime),
           microseconds(usage.ru_stime),
           usage.ru_minflt);
    printf("usage_collected=yes\n");
    return EXIT_SUCCESS;
}
