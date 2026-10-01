/*
 * Exercise 05.13 — Observe without reaping using waitid()
 *
 * Purpose:
 *   Inspect a terminated child with WNOWAIT, leaving the wait status available
 *   for a later waitpid() call.
 *
 * Linux behavior:
 *   waitid() returns structured siginfo_t data. WNOWAIT separates observation
 *   from consumption; the child remains a zombie until a later wait reaps it.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t child = fork();
    siginfo_t information;
    int status;
    pid_t reaped;

    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        _exit(33);
    }

    information.si_pid = 0;
    if (waitid(P_PID, (id_t)child, &information, WEXITED | WNOWAIT) == -1) {
        perror("waitid");
        return EXIT_FAILURE;
    }
    reaped = waitpid(child, &status, 0);
    if (reaped == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("observed_pid_matches=%s observed_status=%d reaped_same=%s\n",
           information.si_pid == child ? "yes" : "no",
           information.si_code == CLD_EXITED ? information.si_status : -1,
           reaped == child ? "yes" : "no");
    return EXIT_SUCCESS;
}
