/*
 * Exercise 10.03 - Suspend until a signal is handled
 *
 * Purpose:
 *   Use pause() to sleep until a child sends SIGUSR1 to the parent.
 *
 * Linux behavior:
 *   pause() returns only after a caught signal handler returns, reports -1,
 *   and sets errno to EINTR. A production design normally prefers
 *   sigsuspend() because it can change the mask and wait atomically.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t handled;

static void remember_signal(int signal_number)
{
    if (signal_number == SIGUSR1) {
        handled = 1;
    }
}

int main(void)
{
    struct sigaction action = {0};
    const struct timespec delay = {.tv_sec = 0, .tv_nsec = 20000000L};
    pid_t child;
    int pause_result;
    int pause_errno;
    int status;

    action.sa_handler = remember_signal;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("install SIGUSR1 handler");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        (void)nanosleep(&delay, NULL);
        _exit(kill(getppid(), SIGUSR1) == 0 ? 0 : 2);
    }

    errno = 0;
    pause_result = pause();
    pause_errno = errno;

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("pause_returned_minus_one=%s errno_is_eintr=%s handled=%s\n",
           pause_result == -1 ? "yes" : "no",
           pause_errno == EINTR ? "yes" : "no",
           handled == 1 ? "yes" : "no");
    printf("child_exit_ok=%s sigsuspend_preferred_for_atomic_wait=yes\n",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no");

    return pause_result == -1 && pause_errno == EINTR && handled == 1 &&
                   WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
