/*
 * Exercise 11.14 - Recover remaining sleep time after EINTR
 *
 * Purpose:
 *   Interrupt nanosleep() with SIGUSR1 and inspect the unslept interval
 *   returned by the kernel.
 *
 * Linux behavior:
 *   A caught signal can make nanosleep() fail with EINTR. The remaining
 *   timespec allows a caller to resume a relative sleep, though repeated
 *   restarts may accumulate scheduling drift.
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
    const struct timespec request = {.tv_sec = 0, .tv_nsec = 100000000L};
    const struct timespec child_delay = {.tv_sec = 0, .tv_nsec = 20000000L};
    struct timespec remaining = {0};
    pid_t child;
    int sleep_result;
    int sleep_errno;
    int status;

    action.sa_handler = remember_signal;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        (void)nanosleep(&child_delay, NULL);
        _exit(kill(getppid(), SIGUSR1) == 0 ? 0 : 2);
    }

    errno = 0;
    sleep_result = nanosleep(&request, &remaining);
    sleep_errno = errno;
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("interrupted=%s errno_is_eintr=%s handler_ran=%s\n",
           sleep_result == -1 ? "yes" : "no",
           sleep_errno == EINTR ? "yes" : "no",
           handled == 1 ? "yes" : "no");
    printf("remaining_positive=%s child_exit_ok=%s\n",
           remaining.tv_sec > (time_t)0 || remaining.tv_nsec > 0L ? "yes" : "no",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no");

    return sleep_result == -1 && sleep_errno == EINTR && handled == 1 &&
                   (remaining.tv_sec > (time_t)0 || remaining.tv_nsec > 0L) &&
                   WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
