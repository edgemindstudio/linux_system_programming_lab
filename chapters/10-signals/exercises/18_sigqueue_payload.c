/*
 * Exercise 10.18 - Send an integer payload with sigqueue()
 *
 * Purpose:
 *   Attach a small application value to SIGUSR1 and retrieve it from
 *   siginfo_t in an SA_SIGINFO handler.
 *
 * Linux behavior:
 *   sigqueue() reports SI_QUEUE and supplies the union sigval payload. Signals
 *   remain a constrained notification mechanism; they are not a replacement
 *   for a pipe, socket, or shared-memory protocol.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t payload_value;
static volatile sig_atomic_t queued_origin;

static void receive_payload(int signal_number, siginfo_t *information,
                            void *context)
{
    (void)context;
    if (signal_number == SIGUSR1) {
        payload_value = information->si_value.sival_int;
        queued_origin = information->si_code == SI_QUEUE ? 1 : 0;
    }
}

int main(void)
{
    struct sigaction action = {0};
    union sigval payload = {.sival_int = 42};

    action.sa_sigaction = receive_payload;
    action.sa_flags = SA_SIGINFO;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("install payload handler");
        return EXIT_FAILURE;
    }
    if (sigqueue(getpid(), SIGUSR1, payload) == -1) {
        perror("sigqueue");
        return EXIT_FAILURE;
    }

    printf("payload_received=%s value=%d origin_is_sigqueue=%s\n",
           payload_value == 42 ? "yes" : "no",
           (int)payload_value,
           queued_origin == 1 ? "yes" : "no");
    printf("signal_is_notification_not_bulk_transport=yes\n");

    return payload_value == 42 && queued_origin == 1
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
