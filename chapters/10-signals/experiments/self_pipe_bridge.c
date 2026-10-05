/*
 * Experiment - Bridge asynchronous delivery into an event loop
 *
 * Prediction:
 *   A minimal handler writes one byte to a nonblocking pipe. poll() then makes
 *   the notification available to ordinary code where complex work is safe.
 *
 * Observation boundary:
 *   Pipe capacity is finite. Production code must define an overflow policy,
 *   commonly by treating any readable byte as a request to rescan state.
 */

#define _GNU_SOURCE

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int signal_pipe[2] = {-1, -1};

static void notify_pipe(int signal_number)
{
    const unsigned char byte = (unsigned char)signal_number;
    ssize_t write_result;

    write_result = write(signal_pipe[1], &byte, sizeof(byte));
    (void)write_result;
}

int main(void)
{
    struct sigaction action = {0};
    struct pollfd descriptor;
    unsigned char received = 0U;
    int poll_result;
    ssize_t read_result;

    if (pipe2(signal_pipe, O_NONBLOCK | O_CLOEXEC) == -1) {
        perror("pipe2");
        return EXIT_FAILURE;
    }
    action.sa_handler = notify_pipe;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("install self-pipe handler");
        return EXIT_FAILURE;
    }
    if (raise(SIGUSR1) != 0) {
        perror("raise SIGUSR1");
        return EXIT_FAILURE;
    }

    descriptor.fd = signal_pipe[0];
    descriptor.events = POLLIN;
    descriptor.revents = 0;
    poll_result = poll(&descriptor, 1U, 1000);
    read_result = read(signal_pipe[0], &received, sizeof(received));

    (void)close(signal_pipe[0]);
    (void)close(signal_pipe[1]);

    printf("poll_reported_readable=%s byte_received=%s signal_matches=%s\n",
           poll_result == 1 && (descriptor.revents & POLLIN) != 0 ? "yes" : "no",
           read_result == 1 ? "yes" : "no",
           received == (unsigned char)SIGUSR1 ? "yes" : "no");
    printf("handler_used_only_async_safe_write=yes complex_work_in_main=yes\n");

    return poll_result == 1 && (descriptor.revents & POLLIN) != 0 &&
                   read_result == 1 && received == (unsigned char)SIGUSR1
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
