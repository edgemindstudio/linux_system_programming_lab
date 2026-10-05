/*
 * Experiment - Consume a signal through Linux signalfd()
 *
 * Prediction:
 *   A blocked SIGUSR1 becomes readable from a file descriptor and carries
 *   structured sender information without invoking a traditional handler.
 *
 * Observation boundary:
 *   signalfd() is Linux-specific. The selected signals must remain blocked so
 *   normal asynchronous delivery does not race with descriptor consumption.
 */

#define _GNU_SOURCE

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <unistd.h>

int main(void)
{
    sigset_t selected;
    sigset_t previous_mask;
    struct signalfd_siginfo information;
    int signal_fd;
    ssize_t received;

    if (sigemptyset(&selected) == -1 ||
        sigaddset(&selected, SIGUSR1) == -1 ||
        sigprocmask(SIG_BLOCK, &selected, &previous_mask) == -1) {
        perror("block SIGUSR1");
        return EXIT_FAILURE;
    }
    signal_fd = signalfd(-1, &selected, SFD_CLOEXEC);
    if (signal_fd == -1) {
        perror("signalfd");
        return EXIT_FAILURE;
    }
    if (kill(getpid(), SIGUSR1) == -1) {
        perror("kill self");
        return EXIT_FAILURE;
    }

    received = read(signal_fd, &information, sizeof(information));
    (void)close(signal_fd);
    if (sigprocmask(SIG_SETMASK, &previous_mask, NULL) == -1) {
        perror("restore signal mask");
        return EXIT_FAILURE;
    }

    printf("record_complete=%s signal_is_sigusr1=%s sender_pid_matches=%s\n",
           received == (ssize_t)sizeof(information) ? "yes" : "no",
           information.ssi_signo == (uint32_t)SIGUSR1 ? "yes" : "no",
           information.ssi_pid == (uint32_t)getpid() ? "yes" : "no");
    printf("linux_specific=yes traditional_handler_invoked=no "
           "signal_was_blocked=yes\n");

    return received == (ssize_t)sizeof(information) &&
                   information.ssi_signo == (uint32_t)SIGUSR1 &&
                   information.ssi_pid == (uint32_t)getpid()
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
