/*
 * Exercise 06.17 - Resource limits survive exec()
 *
 * Purpose:
 *   Configure RLIMIT_NOFILE in a child, replace the child's program image,
 *   and verify that the new image sees the configured value.
 *
 * Linux behavior:
 *   exec() replaces code, data, heap, and stack mappings, but resource limits
 *   are process attributes and remain in force across the replacement.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

static int report_mode(void)
{
    struct rlimit limit;

    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        return EXIT_FAILURE;
    }
    printf("%llu\n", (unsigned long long)limit.rlim_cur);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    struct rlimit original;
    struct rlimit configured;
    unsigned long long observed = 0U;
    int channel[2];
    int status;
    pid_t child;
    FILE *stream;

    if (argc == 2 && argv[1][0] == '-' && argv[1][1] == '-' &&
        argv[1][2] == 'r') {
        return report_mode();
    }

    if (getrlimit(RLIMIT_NOFILE, &original) == -1) {
        perror("getrlimit");
        return EXIT_FAILURE;
    }
    configured = original;
    if (configured.rlim_cur == RLIM_INFINITY || configured.rlim_cur > 80U) {
        configured.rlim_cur = 80U;
    }
    if (pipe(channel) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        (void)close(channel[0]);
        if (setrlimit(RLIMIT_NOFILE, &configured) == -1 ||
            dup2(channel[1], STDOUT_FILENO) == -1) {
            _exit(2);
        }
        (void)close(channel[1]);
        execl("/proc/self/exe", argv[0], "--report", (char *)NULL);
        _exit(127);
    }

    (void)close(channel[1]);
    stream = fdopen(channel[0], "r");
    if (stream == NULL) {
        perror("fdopen");
        return EXIT_FAILURE;
    }
    if (fscanf(stream, "%llu", &observed) != 1) {
        fputs("failed to parse exec report\n", stderr);
        (void)fclose(stream);
        return EXIT_FAILURE;
    }
    if (fclose(stream) == EOF) {
        perror("fclose");
        return EXIT_FAILURE;
    }
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("configured=%llu exec_observed=%llu\n",
           (unsigned long long)configured.rlim_cur, observed);
    printf("limit_preserved_across_exec=%s child_exit=%d\n",
           observed == (unsigned long long)configured.rlim_cur ? "yes" : "no",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);

    return EXIT_SUCCESS;
}
