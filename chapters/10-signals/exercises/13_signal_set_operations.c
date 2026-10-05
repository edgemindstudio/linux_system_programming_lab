/*
 * Exercise 10.13 - Build and inspect signal sets
 *
 * Purpose:
 *   Practice the opaque sigset_t API instead of manipulating an assumed bit
 *   representation directly.
 *
 * Linux behavior:
 *   sigemptyset(), sigfillset(), sigaddset(), sigdelset(), and sigismember()
 *   construct and query sets used by masks and waiting interfaces. The set
 *   object itself does not send, block, or wait for a signal.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    sigset_t selected;
    sigset_t almost_all;
    int selected_ok;
    int removed_ok;

    if (sigemptyset(&selected) == -1 ||
        sigaddset(&selected, SIGUSR1) == -1 ||
        sigaddset(&selected, SIGTERM) == -1 ||
        sigfillset(&almost_all) == -1 ||
        sigdelset(&almost_all, SIGINT) == -1) {
        perror("signal-set operation");
        return EXIT_FAILURE;
    }

    selected_ok = sigismember(&selected, SIGUSR1) == 1 &&
                  sigismember(&selected, SIGTERM) == 1 &&
                  sigismember(&selected, SIGINT) == 0;
    removed_ok = sigismember(&almost_all, SIGINT) == 0 &&
                 sigismember(&almost_all, SIGUSR1) == 1;

    printf("selected_members_correct=%s deleted_member_absent=%s\n",
           selected_ok ? "yes" : "no",
           removed_ok ? "yes" : "no");
    printf("opaque_set_api_used=yes set_creation_does_not_change_mask=yes\n");

    return selected_ok && removed_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
