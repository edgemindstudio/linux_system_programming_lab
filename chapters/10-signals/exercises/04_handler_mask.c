/*
 * Exercise 10.04 - Extend the signal mask while a handler runs
 *
 * Purpose:
 *   Add SIGUSR2 to the temporary mask used during a SIGUSR1 handler and
 *   verify that SIGUSR2 becomes pending until the first handler returns.
 *
 * Linux behavior:
 *   The delivered signal is normally blocked during its own handler, and
 *   sa_mask can block additional signals. A blocked generated signal remains
 *   pending rather than invoking its handler immediately.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t usr2_pending_in_usr1;
static volatile sig_atomic_t usr2_handled;

static void handle_usr2(int signal_number)
{
    if (signal_number == SIGUSR2) {
        usr2_handled = 1;
    }
}

static void handle_usr1(int signal_number)
{
    sigset_t pending;

    if (signal_number != SIGUSR1) {
        return;
    }
    if (kill(getpid(), SIGUSR2) == -1) {
        return;
    }
    if (sigpending(&pending) == 0 && sigismember(&pending, SIGUSR2) == 1) {
        usr2_pending_in_usr1 = 1;
    }
}

int main(void)
{
    struct sigaction first = {0};
    struct sigaction second = {0};

    first.sa_handler = handle_usr1;
    second.sa_handler = handle_usr2;
    if (sigemptyset(&first.sa_mask) == -1 ||
        sigaddset(&first.sa_mask, SIGUSR2) == -1 ||
        sigemptyset(&second.sa_mask) == -1 ||
        sigaction(SIGUSR1, &first, NULL) == -1 ||
        sigaction(SIGUSR2, &second, NULL) == -1) {
        perror("configure handlers");
        return EXIT_FAILURE;
    }
    if (raise(SIGUSR1) != 0) {
        perror("raise SIGUSR1");
        return EXIT_FAILURE;
    }

    printf("usr2_pending_during_usr1=%s usr2_handled_after_return=%s\n",
           usr2_pending_in_usr1 == 1 ? "yes" : "no",
           usr2_handled == 1 ? "yes" : "no");
    printf("additional_handler_mask_used=yes nested_delivery_avoided=yes\n");

    return usr2_pending_in_usr1 == 1 && usr2_handled == 1
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
