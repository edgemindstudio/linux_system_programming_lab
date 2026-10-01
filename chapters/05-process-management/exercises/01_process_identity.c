/*
 * Exercise 05.01 — Process identity and hierarchy
 *
 * Purpose:
 *   Read the identifiers that place one running program in Linux's process
 *   hierarchy. A PID names this process, a PPID names its current parent, a
 *   process-group ID joins related jobs, and a session ID joins process
 *   groups that share job-control history.
 *
 * Linux behavior:
 *   These values describe the calling process at the instant each function
 *   runs. PID values are reusable after a process is reaped, so a PID is not
 *   a permanent identity. Containers can also show a process different PID
 *   values in nested PID namespaces.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = getpid();
    pid_t parent_pid = getppid();
    pid_t process_group = getpgid(0);
    pid_t session = getsid(0);

    if (process_group == (pid_t)-1) {
        perror("getpgid");
        return EXIT_FAILURE;
    }
    if (session == (pid_t)-1) {
        perror("getsid");
        return EXIT_FAILURE;
    }

    printf("pid=%ld parent=%ld process_group=%ld session=%ld\n",
           (long)pid,
           (long)parent_pid,
           (long)process_group,
           (long)session);
    printf("pid_positive=%s parent_nonnegative=%s\n",
           pid > 0 ? "yes" : "no",
           parent_pid >= 0 ? "yes" : "no");

    return EXIT_SUCCESS;
}
