/*
 * Exercise 10.16 - Accept a signal synchronously with sigwait()
 *
 * Purpose:
 *   Keep SIGUSR1 blocked and turn its arrival into ordinary synchronous
 *   control flow rather than an asynchronous handler invocation.
 *
 * Linux behavior:
 *   sigwait() removes one pending signal from the supplied blocked set and
 *   returns its number through an output parameter. It returns an error number
 *   directly rather than using the usual -1 and errno convention.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    sigset_t wait_set;
    sigset_t previous_mask;
    pid_t child;
    int received = 0;
    int wait_result;
    int status;

    if (sigemptyset(&wait_set) == -1 ||
        sigaddset(&wait_set, SIGUSR1) == -1 ||
        sigprocmask(SIG_BLOCK, &wait_set, &previous_mask) == -1) {
        perror("block SIGUSR1");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        _exit(kill(getppid(), SIGUSR1) == 0 ? 0 : 2);
    }

    wait_result = sigwait(&wait_set, &received);
    if (wait_result != 0) {
        fprintf(stderr, "sigwait: %s\n", strerror(wait_result));
        return EXIT_FAILURE;
    }
    if (sigprocmask(SIG_SETMASK, &previous_mask, NULL) == -1 ||
        waitpid(child, &status, 0) == -1) {
        perror("restore mask or waitpid");
        return EXIT_FAILURE;
    }

    printf("sigwait_succeeded=yes received_sigusr1=%s handler_required=no\n",
           received == SIGUSR1 ? "yes" : "no");
    printf("signal_consumed_synchronously=yes child_exit_ok=%s\n",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no");

    return received == SIGUSR1 && WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
