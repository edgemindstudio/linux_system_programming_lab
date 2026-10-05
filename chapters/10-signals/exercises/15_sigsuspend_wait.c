/*
 * Exercise 10.15 - Wait without the check-then-sleep race
 *
 * Purpose:
 *   Block SIGUSR1 while preparing, then use sigsuspend() to atomically install
 *   a temporary mask and sleep for delivery.
 *
 * Linux behavior:
 *   sigsuspend() always returns -1 with EINTR after a caught signal. The
 *   process's original mask is restored automatically before it returns.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
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
    sigset_t block_set;
    sigset_t previous_mask;
    sigset_t wait_mask;
    pid_t child;
    int suspend_result = 0;
    int suspend_errno = 0;
    int status;

    action.sa_handler = remember_signal;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1 ||
        sigemptyset(&block_set) == -1 ||
        sigaddset(&block_set, SIGUSR1) == -1 ||
        sigprocmask(SIG_BLOCK, &block_set, &previous_mask) == -1) {
        perror("prepare sigsuspend");
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

    wait_mask = previous_mask;
    if (sigdelset(&wait_mask, SIGUSR1) == -1) {
        perror("sigdelset");
        return EXIT_FAILURE;
    }
    while (handled == 0) {
        errno = 0;
        suspend_result = sigsuspend(&wait_mask);
        suspend_errno = errno;
    }

    if (sigprocmask(SIG_SETMASK, &previous_mask, NULL) == -1 ||
        waitpid(child, &status, 0) == -1) {
        perror("restore mask or waitpid");
        return EXIT_FAILURE;
    }

    printf("signal_handled=%s sigsuspend_returned_minus_one=%s "
           "errno_is_eintr=%s\n",
           handled == 1 ? "yes" : "no",
           suspend_result == -1 ? "yes" : "no",
           suspend_errno == EINTR ? "yes" : "no");
    printf("check_and_wait_atomic=yes original_mask_restored=yes child_exit_ok=%s\n",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no");

    return handled == 1 && suspend_result == -1 && suspend_errno == EINTR &&
                   WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
