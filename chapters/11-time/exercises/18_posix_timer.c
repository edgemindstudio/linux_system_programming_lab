/*
 * Exercise 11.18 - Create and consume a POSIX timer
 *
 * Purpose:
 *   Create a CLOCK_MONOTONIC timer that queues a real-time signal carrying an
 *   application value, then accept that signal synchronously.
 *
 * Linux behavior:
 *   POSIX timers are independent timer objects. SIGEV_SIGNAL can attach a
 *   sigval payload, and blocking the signal before arming allows sigwaitinfo()
 *   to consume it without an asynchronous handler.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(void)
{
    const int signal_number = SIGRTMIN;
    sigset_t set;
    struct sigevent event = {0};
    const struct itimerspec specification = {
        .it_interval = {0, 0},
        .it_value = {0, 20000000L},
    };
    siginfo_t information;
    timer_t timer;
    int received;

    if (sigemptyset(&set) == -1 ||
        sigaddset(&set, signal_number) == -1 ||
        sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        perror("block timer signal");
        return EXIT_FAILURE;
    }

    event.sigev_notify = SIGEV_SIGNAL;
    event.sigev_signo = signal_number;
    event.sigev_value.sival_int = 77;
    if (timer_create(CLOCK_MONOTONIC, &event, &timer) == -1) {
        perror("timer_create");
        return EXIT_FAILURE;
    }
    if (timer_settime(timer, 0, &specification, NULL) == -1) {
        perror("timer_settime");
        (void)timer_delete(timer);
        return EXIT_FAILURE;
    }

    memset(&information, 0, sizeof(information));
    received = sigwaitinfo(&set, &information);
    if (received == -1) {
        perror("sigwaitinfo");
        (void)timer_delete(timer);
        return EXIT_FAILURE;
    }
    if (timer_delete(timer) == -1) {
        perror("timer_delete");
        return EXIT_FAILURE;
    }

    printf("timer_signal_received=%s payload=%d payload_matches=%s\n",
           received == signal_number ? "yes" : "no",
           information.si_value.sival_int,
           information.si_value.sival_int == 77 ? "yes" : "no");
    printf("clock=monotonic notification=SIGEV_SIGNAL synchronous_wait=yes deleted=yes\n");

    return received == signal_number && information.si_value.sival_int == 77
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
