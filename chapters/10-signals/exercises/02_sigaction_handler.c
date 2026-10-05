/*
 * Exercise 10.02 - Install a signal handler with sigaction()
 *
 * Purpose:
 *   Replace SIGUSR1's default disposition, generate the signal, and confirm
 *   that the registered handler ran.
 *
 * Linux behavior:
 *   Delivery temporarily diverts control to the handler. The handler changes
 *   only a volatile sig_atomic_t object; normal formatted output remains in
 *   main because stdio functions are not async-signal-safe.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static volatile sig_atomic_t handled;

static void remember_signal(int signal_number)
{
    handled = signal_number == SIGUSR1 ? 1 : -1;
}

int main(void)
{
    struct sigaction action = {0};

    action.sa_handler = remember_signal;
    if (sigemptyset(&action.sa_mask) == -1) {
        perror("sigemptyset");
        return EXIT_FAILURE;
    }
    if (sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (raise(SIGUSR1) != 0) {
        perror("raise");
        return EXIT_FAILURE;
    }

    printf("handler_installed=yes signal_handled=%s\n",
           handled == 1 ? "yes" : "no");
    printf("handler_used_stdio=no main_performed_output=yes\n");

    return handled == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
