/*
 * Experiment - Consume timer expirations through a file descriptor
 *
 * Prediction:
 *   A periodic timerfd accumulates expirations in a 64-bit counter and can be
 *   integrated with ordinary read/poll/epoll event loops.
 */

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

int main(void)
{
    const struct itimerspec specification = {
        .it_interval = {0, 5000000L},
        .it_value = {0, 5000000L},
    };
    const struct timespec delay = {.tv_sec = 0, .tv_nsec = 28000000L};
    uint64_t expirations = 0U;
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    ssize_t received;

    if (fd == -1) {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }
    if (timerfd_settime(fd, 0, &specification, NULL) == -1) {
        perror("timerfd_settime");
        (void)close(fd);
        return EXIT_FAILURE;
    }

    (void)nanosleep(&delay, NULL);
    received = read(fd, &expirations, sizeof(expirations));
    if (received != (ssize_t)sizeof(expirations)) {
        perror("read timerfd");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close timerfd");
        return EXIT_FAILURE;
    }

    printf("counter_bytes=%zd expirations_positive=%s multiple_expirations_observed=%s\n",
           received,
           expirations > 0U ? "yes" : "no",
           expirations > 1U ? "yes" : "no");
    printf("event_loop_compatible=yes signal_handler_required=no\n");

    return expirations > 0U ? EXIT_SUCCESS : EXIT_FAILURE;
}
