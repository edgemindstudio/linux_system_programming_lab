/*
 * Exercise 10.09 - Send a terminating signal to a child
 *
 * Purpose:
 *   Deliver SIGTERM with kill() and decode the child's wait status.
 *
 * Linux behavior:
 *   SIGTERM's default disposition terminates the target. waitpid() reports a
 *   signal termination separately from a normal exit status.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int ready_pipe[2];
    pid_t child;
    int status;
    char ready;

    if (pipe(ready_pipe) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        char marker = 'r';

        (void)close(ready_pipe[0]);
        if (write(ready_pipe[1], &marker, 1U) != 1) {
            _exit(2);
        }
        (void)close(ready_pipe[1]);
        for (;;) {
            pause();
        }
    }

    (void)close(ready_pipe[1]);
    if (read(ready_pipe[0], &ready, 1U) != 1) {
        perror("read ready");
        return EXIT_FAILURE;
    }
    (void)close(ready_pipe[0]);
    if (kill(child, SIGTERM) == -1) {
        perror("kill SIGTERM");
        return EXIT_FAILURE;
    }
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("kill_succeeded=yes child_signaled=%s terminating_signal_is_sigterm=%s\n",
           WIFSIGNALED(status) ? "yes" : "no",
           WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM ? "yes" : "no");
    printf("child_reaped=yes unrelated_processes_signaled=no\n");

    return WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
