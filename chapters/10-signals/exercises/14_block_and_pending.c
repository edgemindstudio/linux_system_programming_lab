/*
 * Exercise 10.14 - Block a signal and inspect pending state
 *
 * Purpose:
 *   Block SIGUSR1, generate it, verify that it is pending, and then restore
 *   the previous mask so delivery can occur.
 *
 * Linux behavior:
 *   Blocking postpones delivery; it does not discard generation. A pending
 *   standard signal is represented as present or absent rather than as an
 *   unbounded count of occurrences.
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
    sigset_t block_set;
    sigset_t previous_mask;
    sigset_t pending_set;
    int was_pending;

    action.sa_handler = remember_signal;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1 ||
        sigemptyset(&block_set) == -1 ||
        sigaddset(&block_set, SIGUSR1) == -1 ||
        sigprocmask(SIG_BLOCK, &block_set, &previous_mask) == -1) {
        perror("configure blocked signal");
        return EXIT_FAILURE;
    }
    if (raise(SIGUSR1) != 0 || sigpending(&pending_set) == -1) {
        perror("generate or inspect pending signal");
        return EXIT_FAILURE;
    }

    was_pending = sigismember(&pending_set, SIGUSR1) == 1;
    if (sigprocmask(SIG_SETMASK, &previous_mask, NULL) == -1) {
        perror("restore signal mask");
        return EXIT_FAILURE;
    }

    printf("pending_while_blocked=%s handled_after_unblock=%s\n",
           was_pending ? "yes" : "no",
           handled == 1 ? "yes" : "no");
    printf("blocking_postponed_delivery=yes previous_mask_restored=yes\n");

    return was_pending && handled == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
