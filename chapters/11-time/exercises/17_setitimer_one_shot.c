/*
 * Exercise 11.17 - Arm a subsecond interval timer
 *
 * Purpose:
 *   Configure ITIMER_REAL as a one-shot timer and wait for SIGALRM.
 *
 * Linux behavior:
 *   setitimer() accepts timeval precision and supports one-shot or periodic
 *   expiration. ITIMER_REAL measures wall time and generates SIGALRM.
 */

#define _DEFAULT_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

static volatile sig_atomic_t expired;

static void remember_expiration(int signal_number)
{
    if (signal_number == SIGALRM) {
        expired = 1;
    }
}

int main(void)
{
    struct sigaction action = {0};
    const struct itimerval timer = {
        .it_interval = {0, 0},
        .it_value = {0, 20000},
    };

    action.sa_handler = remember_expiration;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGALRM, &action, NULL) == -1) {
        perror("sigaction SIGALRM");
        return EXIT_FAILURE;
    }
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        return EXIT_FAILURE;
    }
    while (expired == 0) {
        (void)pause();
    }

    printf("timer_kind=ITIMER_REAL one_shot=yes expired=%s\n",
           expired == 1 ? "yes" : "no");
    printf("requested_microseconds=20000 delivered_signal=SIGALRM\n");

    return expired == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
