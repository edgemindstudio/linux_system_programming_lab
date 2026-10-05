/*
 * Exercise 10.10 - Send a signal to the calling process
 *
 * Purpose:
 *   Use raise() as the process-local counterpart of sending a signal with
 *   kill().
 *
 * Linux behavior:
 *   raise(SIGUSR1) generates SIGUSR1 for the caller. With the signal unblocked,
 *   the handler runs before raise() returns successfully.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

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
    int result;

    action.sa_handler = remember_signal;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("install handler");
        return EXIT_FAILURE;
    }

    result = raise(SIGUSR1);
    printf("raise_result_zero=%s handler_completed_before_return=%s\n",
           result == 0 ? "yes" : "no",
           handled == 1 ? "yes" : "no");
    printf("target_is_calling_process=yes symbolic_identifier_used=yes\n");

    return result == 0 && handled == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
