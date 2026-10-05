/*
 * Exercise 10.17 - Inspect sender metadata with SA_SIGINFO
 *
 * Purpose:
 *   Install the three-argument handler form and inspect the metadata Linux
 *   supplies for a signal sent with kill().
 *
 * Linux behavior:
 *   SA_SIGINFO selects a handler that receives siginfo_t and a context pointer.
 *   For a user-generated kill(), si_pid identifies the sending process and
 *   si_code identifies a user-originated signal.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t received_signal;
static volatile sig_atomic_t sender_matches;
static volatile sig_atomic_t user_origin;

static void inspect_signal(int signal_number, siginfo_t *information,
                           void *context)
{
    (void)context;
    received_signal = signal_number;
    sender_matches = information->si_pid == getpid() ? 1 : 0;
    user_origin = information->si_code == SI_USER ? 1 : 0;
}

int main(void)
{
    struct sigaction action = {0};

    action.sa_sigaction = inspect_signal;
    action.sa_flags = SA_SIGINFO;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("install SA_SIGINFO handler");
        return EXIT_FAILURE;
    }
    if (kill(getpid(), SIGUSR1) == -1) {
        perror("kill self");
        return EXIT_FAILURE;
    }

    printf("received_sigusr1=%s sender_pid_matches=%s user_origin=%s\n",
           received_signal == SIGUSR1 ? "yes" : "no",
           sender_matches == 1 ? "yes" : "no",
           user_origin == 1 ? "yes" : "no");
    printf("sa_sigaction_used=yes context_pointer_ignored_safely=yes\n");

    return received_signal == SIGUSR1 && sender_matches == 1 && user_origin == 1
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
