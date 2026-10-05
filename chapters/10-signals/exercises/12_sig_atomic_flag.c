/*
 * Exercise 10.12 - Communicate with a minimal handler flag
 *
 * Purpose:
 *   Use volatile sig_atomic_t as the narrow communication channel between an
 *   asynchronous handler and ordinary control flow.
 *
 * Linux behavior:
 *   The handler may interrupt main between ordinary instructions. Assigning a
 *   value to a volatile sig_atomic_t object avoids a torn access and prevents
 *   the compiler from treating the flag as unchanging across the handler.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static volatile sig_atomic_t stop_requested;

static void request_stop(int signal_number)
{
    if (signal_number == SIGTERM) {
        stop_requested = 1;
    }
}

int main(void)
{
    struct sigaction action = {0};

    action.sa_handler = request_stop;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGTERM, &action, NULL) == -1) {
        perror("install SIGTERM handler");
        return EXIT_FAILURE;
    }
    if (raise(SIGTERM) != 0) {
        perror("raise SIGTERM");
        return EXIT_FAILURE;
    }

    printf("stop_requested=%s volatile_sig_atomic_t_used=yes\n",
           stop_requested == 1 ? "yes" : "no");
    printf("complex_cleanup_in_handler=no cleanup_belongs_in_main=yes\n");

    return stop_requested == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
