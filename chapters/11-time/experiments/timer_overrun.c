/*
 * Experiment - Observe POSIX timer overruns
 *
 * Prediction:
 *   If a periodic timer expires repeatedly while its notification signal is
 *   still pending, Linux records additional expirations as an overrun count.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    const int signal_number = SIGRTMIN;
    const struct itimerspec specification = {
        .it_interval = {0, 1000000L},
        .it_value = {0, 1000000L},
    };
    const struct timespec delay = {.tv_sec = 0, .tv_nsec = 25000000L};
    struct sigevent event = {0};
    sigset_t set;
    siginfo_t information;
    timer_t timer;
    int received;
    int overrun;

    if (sigemptyset(&set) == -1 || sigaddset(&set, signal_number) == -1 ||
        sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        perror("block signal");
        return EXIT_FAILURE;
    }
    event.sigev_notify = SIGEV_SIGNAL;
    event.sigev_signo = signal_number;
    if (timer_create(CLOCK_MONOTONIC, &event, &timer) == -1 ||
        timer_settime(timer, 0, &specification, NULL) == -1) {
        perror("configure timer");
        return EXIT_FAILURE;
    }

    (void)nanosleep(&delay, NULL);
    received = sigwaitinfo(&set, &information);
    if (received == -1) {
        perror("sigwaitinfo");
        (void)timer_delete(timer);
        return EXIT_FAILURE;
    }
    overrun = timer_getoverrun(timer);
    if (overrun == -1) {
        perror("timer_getoverrun");
        (void)timer_delete(timer);
        return EXIT_FAILURE;
    }
    if (timer_delete(timer) == -1) {
        perror("timer_delete");
        return EXIT_FAILURE;
    }

    printf("signal_received=%s overrun_nonnegative=%s overrun_positive=%s\n",
           received == signal_number ? "yes" : "no",
           overrun >= 0 ? "yes" : "no",
           overrun > 0 ? "yes" : "no");
    printf("one_notification_can_represent_multiple_expirations=yes\n");

    return received == signal_number && overrun >= 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
