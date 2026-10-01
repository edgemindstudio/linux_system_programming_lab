/*
 * Exercise 05.10 — Decode signal termination
 *
 * Purpose:
 *   Terminate a child with SIGTERM and distinguish that event from a normal
 *   numeric exit code.
 *
 * Linux behavior:
 *   WIFSIGNALED() must be true before WTERMSIG() is meaningful. Signal
 *   termination and exit(128 + signal) are not the same kernel wait state,
 *   even though shells often present related numeric values.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t child = fork();
    int status;

    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        if (raise(SIGTERM) != 0) {
            _exit(120);
        }
        _exit(121);
    }

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("signaled=%s signal_matches_SIGTERM=%s normal_exit=%s\n",
           WIFSIGNALED(status) ? "yes" : "no",
           WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM ? "yes" : "no",
           WIFEXITED(status) ? "yes" : "no");
    return EXIT_SUCCESS;
}
