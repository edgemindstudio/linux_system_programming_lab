/*
 * Exercise 05.09 — Decode a child's normal exit status
 *
 * Purpose:
 *   Reap one child with waitpid() and use the status macros in the required
 *   order: test WIFEXITED() before reading WEXITSTATUS().
 *
 * Linux behavior:
 *   The wait status is an encoded integer, not the exit code itself. Only the
 *   low eight bits of a normal exit value are available to the parent.
 */

#define _POSIX_C_SOURCE 200809L

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
        _exit(42);
    }

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("child=%ld exited=%s exit_status=%d\n",
           (long)child,
           WIFEXITED(status) ? "yes" : "no",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    return EXIT_SUCCESS;
}
