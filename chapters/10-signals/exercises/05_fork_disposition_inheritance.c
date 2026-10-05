/*
 * Exercise 10.05 - Signal dispositions cross fork()
 *
 * Purpose:
 *   Install a SIGUSR1 handler, fork, and show that the child begins with the
 *   same caught disposition as its parent.
 *
 * Linux behavior:
 *   A child inherits its parent's signal dispositions and mask. Pending
 *   signals are not inherited. After the fork, each process has its own copy
 *   of the sig_atomic_t flag changed by the inherited handler.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t handled;

static void remember_signal(int signal_number)
{
    if (signal_number == SIGUSR1) {
        handled = 1;
    }
}

int main(void)
{
    struct sigaction action = {0};
    pid_t child;
    int status;

    action.sa_handler = remember_signal;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("install handler");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        if (raise(SIGUSR1) != 0) {
            _exit(2);
        }
        _exit(handled == 1 ? 0 : 3);
    }

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("child_inherited_handler=%s child_exit_ok=%s\n",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no");
    printf("parent_flag_unchanged=%s pending_signals_not_inherited=yes\n",
           handled == 0 ? "yes" : "no");

    return WIFEXITED(status) && WEXITSTATUS(status) == 0 && handled == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
