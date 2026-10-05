/*
 * Exercise 10.08 - Probe process existence and permission with signal zero
 *
 * Purpose:
 *   Use kill(pid, 0) to validate a process target without delivering a
 *   signal.
 *
 * Linux behavior:
 *   The kernel performs PID lookup and permission checks for signal zero but
 *   does not enqueue a signal. Success is a momentary observation; the target
 *   can still exit immediately afterward.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int release_pipe[2];
    pid_t child;
    int child_probe;
    int self_probe;
    int status;
    char release = 'x';

    if (pipe(release_pipe) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        char byte;

        (void)close(release_pipe[1]);
        _exit(read(release_pipe[0], &byte, 1U) == 1 ? 0 : 2);
    }

    (void)close(release_pipe[0]);
    child_probe = kill(child, 0);
    self_probe = kill(getpid(), 0);
    if (write(release_pipe[1], &release, 1U) != 1) {
        perror("write release");
        return EXIT_FAILURE;
    }
    (void)close(release_pipe[1]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("child_probe_succeeded=%s self_probe_succeeded=%s\n",
           child_probe == 0 ? "yes" : "no",
           self_probe == 0 ? "yes" : "no");
    printf("signal_delivered=no observation_is_race_prone=yes child_exit_ok=%s\n",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no");

    return child_probe == 0 && self_probe == 0 && WIFEXITED(status) &&
                   WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
