/*
 * Experiment - Standard signals coalesce while blocked
 *
 * Prediction:
 *   Sending the same standard signal repeatedly while it is blocked records
 *   one pending condition, not eight independently queued occurrences.
 *
 * Observation boundary:
 *   This is the Linux/POSIX standard-signal model. It must not be generalized
 *   to real-time signals, which are queued and tested separately.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum { SEND_COUNT = 8 };

static volatile sig_atomic_t handler_count;

static void count_delivery(int signal_number)
{
    if (signal_number == SIGUSR1) {
        handler_count += 1;
    }
}

int main(void)
{
    struct sigaction action = {0};
    sigset_t block_set;
    sigset_t previous_mask;
    sigset_t pending_set;
    int pending;

    action.sa_handler = count_delivery;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1 ||
        sigemptyset(&block_set) == -1 ||
        sigaddset(&block_set, SIGUSR1) == -1 ||
        sigprocmask(SIG_BLOCK, &block_set, &previous_mask) == -1) {
        perror("prepare coalescing experiment");
        return EXIT_FAILURE;
    }

    for (int index = 0; index < SEND_COUNT; ++index) {
        if (kill(getpid(), SIGUSR1) == -1) {
            perror("kill SIGUSR1");
            return EXIT_FAILURE;
        }
    }
    if (sigpending(&pending_set) == -1) {
        perror("sigpending");
        return EXIT_FAILURE;
    }
    pending = sigismember(&pending_set, SIGUSR1) == 1;
    if (sigprocmask(SIG_SETMASK, &previous_mask, NULL) == -1) {
        perror("restore signal mask");
        return EXIT_FAILURE;
    }

    printf("signals_sent=%d pending_bit_set=%s handler_invocations=%d\n",
           SEND_COUNT,
           pending ? "yes" : "no",
           (int)handler_count);
    printf("standard_signal_coalesced=%s occurrences_not_counted=yes\n",
           handler_count == 1 ? "yes" : "no");

    return pending && handler_count == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
