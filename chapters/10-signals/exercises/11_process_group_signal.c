/*
 * Exercise 10.11 - Send a signal to an isolated process group
 *
 * Purpose:
 *   Place a child in a new process group and use a negative kill() target to
 *   signal every member of that group.
 *
 * Linux behavior:
 *   kill(-pgid, signal) targets a process group. Isolating the child first is
 *   essential: signaling the shell's foreground group would affect unrelated
 *   processes, including the test driver.
 */

#define _POSIX_C_SOURCE 200809L

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
    int ready_pipe[2];
    pid_t child;
    int status;
    char ready;

    if (pipe(ready_pipe) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        struct sigaction action = {0};
        sigset_t wait_mask;
        char marker = 'r';

        (void)close(ready_pipe[0]);
        if (setpgid(0, 0) == -1) {
            _exit(2);
        }
        action.sa_handler = remember_signal;
        if (sigemptyset(&action.sa_mask) == -1 ||
            sigaction(SIGUSR1, &action, NULL) == -1 ||
            sigemptyset(&wait_mask) == -1) {
            _exit(3);
        }
        if (write(ready_pipe[1], &marker, 1U) != 1) {
            _exit(4);
        }
        (void)close(ready_pipe[1]);
        while (handled == 0) {
            (void)sigsuspend(&wait_mask);
        }
        _exit(0);
    }

    (void)close(ready_pipe[1]);
    if (read(ready_pipe[0], &ready, 1U) != 1) {
        perror("read ready");
        return EXIT_FAILURE;
    }
    (void)close(ready_pipe[0]);
    if (kill(-child, SIGUSR1) == -1) {
        perror("kill process group");
        return EXIT_FAILURE;
    }
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("isolated_group_created=yes group_signal_sent=yes child_handled=%s\n",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no");
    printf("shell_group_untouched=yes negative_pid_semantics_used=yes\n");

    return WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
