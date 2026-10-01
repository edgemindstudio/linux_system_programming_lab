/*
 * Exercise 05.12 — Poll a child with WNOHANG
 *
 * Purpose:
 *   Ask for child status without blocking, then release the child and perform
 *   the final blocking reap.
 *
 * Linux behavior:
 *   waitpid() returns zero with WNOHANG when matching children exist but none
 *   has a reportable state change. Zero is not an error and not a child PID.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int gate[2];
    pid_t child;
    pid_t first_result;
    int status;
    char token = 'R';

    if (pipe(gate) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        char received;
        (void)close(gate[1]);
        if (read(gate[0], &received, 1U) != 1) {
            _exit(120);
        }
        _exit(7);
    }

    (void)close(gate[0]);
    first_result = waitpid(child, &status, WNOHANG);
    if (first_result == -1) {
        perror("waitpid WNOHANG");
        return EXIT_FAILURE;
    }
    if (write(gate[1], &token, 1U) != 1) {
        perror("write gate");
        return EXIT_FAILURE;
    }
    (void)close(gate[1]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid final");
        return EXIT_FAILURE;
    }

    printf("initial_running=%s final_exit=%d\n",
           first_result == 0 ? "yes" : "no",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    return EXIT_SUCCESS;
}
