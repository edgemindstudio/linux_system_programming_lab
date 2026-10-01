/*
 * Exercise 05.11 — Wait for a particular child
 *
 * Purpose:
 *   Create two children and reap them in a chosen order with waitpid(pid,...),
 *   independently of the scheduler's order.
 *
 * Linux behavior:
 *   A terminated child remains waitable as a zombie until its parent collects
 *   the status. Waiting for one child does not discard another child's status.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int exit_code(int status)
{
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int main(void)
{
    int gate[2];
    pid_t slow_child;
    pid_t fast_child;
    int fast_status;
    int slow_status;
    char release = 'R';

    if (pipe(gate) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    slow_child = fork();
    if (slow_child == -1) {
        perror("fork slow child");
        return EXIT_FAILURE;
    }
    if (slow_child == 0) {
        char token;
        (void)close(gate[1]);
        if (read(gate[0], &token, 1U) != 1) {
            _exit(120);
        }
        _exit(11);
    }

    fast_child = fork();
    if (fast_child == -1) {
        perror("fork fast child");
        return EXIT_FAILURE;
    }
    if (fast_child == 0) {
        (void)close(gate[0]);
        (void)close(gate[1]);
        _exit(22);
    }

    (void)close(gate[0]);
    if (waitpid(fast_child, &fast_status, 0) == -1) {
        perror("waitpid fast child");
        return EXIT_FAILURE;
    }
    if (write(gate[1], &release, 1U) != 1) {
        perror("release slow child");
        return EXIT_FAILURE;
    }
    (void)close(gate[1]);
    if (waitpid(slow_child, &slow_status, 0) == -1) {
        perror("waitpid slow child");
        return EXIT_FAILURE;
    }

    printf("first_status=%d second_status=%d specific_order=%s\n",
           exit_code(fast_status),
           exit_code(slow_status),
           exit_code(fast_status) == 22 && exit_code(slow_status) == 11
               ? "yes"
               : "no");
    return EXIT_SUCCESS;
}
