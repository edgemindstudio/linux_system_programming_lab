/*
 * Exercise 06.16 - Resource limits cross fork()
 *
 * Purpose:
 *   Lower a soft descriptor limit, fork, and compare the child's observation
 *   with the parent's configured value.
 *
 * Linux behavior:
 *   Resource limits are copied into the child at fork(). Later changes in one
 *   process do not rewrite the other's independent limit state.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    struct rlimit original;
    struct rlimit configured;
    rlim_t child_soft = RLIM_INFINITY;
    int channel[2];
    int status;
    pid_t child;
    ssize_t transferred;

    if (getrlimit(RLIMIT_NOFILE, &original) == -1) {
        perror("getrlimit original");
        return EXIT_FAILURE;
    }
    configured = original;
    if (configured.rlim_cur == RLIM_INFINITY || configured.rlim_cur > 96U) {
        configured.rlim_cur = 96U;
    }
    if (setrlimit(RLIMIT_NOFILE, &configured) == -1) {
        perror("setrlimit configured");
        return EXIT_FAILURE;
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
        struct rlimit observed;

        (void)close(channel[0]);
        if (getrlimit(RLIMIT_NOFILE, &observed) == -1) {
            _exit(2);
        }
        transferred = write(channel[1], &observed.rlim_cur,
                            sizeof(observed.rlim_cur));
        (void)close(channel[1]);
        _exit(transferred == (ssize_t)sizeof(observed.rlim_cur) ? 0 : 3);
    }

    (void)close(channel[1]);
    transferred = read(channel[0], &child_soft, sizeof(child_soft));
    (void)close(channel[0]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }
    if (setrlimit(RLIMIT_NOFILE, &original) == -1) {
        perror("restore parent limit");
        return EXIT_FAILURE;
    }

    printf("parent_configured=%llu child_observed=%llu\n",
           (unsigned long long)configured.rlim_cur,
           (unsigned long long)child_soft);
    printf("limit_inherited=%s child_exit=%d parent_restored=yes\n",
           transferred == (ssize_t)sizeof(child_soft) &&
                   child_soft == configured.rlim_cur
               ? "yes" : "no",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);

    return EXIT_SUCCESS;
}
