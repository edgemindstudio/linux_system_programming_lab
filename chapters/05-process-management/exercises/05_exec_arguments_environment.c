/*
 * Exercise 05.05 — Arguments and environment across execve()
 *
 * Purpose:
 *   Replace this program with a second instance of itself while supplying a
 *   deliberate argument vector and a small, controlled environment.
 *
 * Linux behavior:
 *   execve() keeps the process identity but replaces code, data, heap, and
 *   stack. The new program receives fresh argv and environ arrays constructed
 *   from the pointers passed to execve(). /proc/self/exe is Linux-specific and
 *   names the executable for the calling process.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int child_mode(int argc, char *argv[])
{
    const char *mode = getenv("LAB_MODE");
    const char *original_pid = getenv("ORIGINAL_PID");
    char current_pid[32];

    if (snprintf(current_pid, sizeof(current_pid), "%ld", (long)getpid()) < 0) {
        return EXIT_FAILURE;
    }

    printf("argc=%d arg1=%s arg2=%s env=%s same_pid=%s\n",
           argc,
           argc > 1 ? argv[1] : "missing",
           argc > 2 ? argv[2] : "missing",
           mode != NULL ? mode : "missing",
           original_pid != NULL && strcmp(original_pid, current_pid) == 0
               ? "yes"
               : "no");
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    char pid_text[32];
    char pid_environment[64];
    char *child_argv[] = {
        "05_exec_arguments_environment",
        "alpha",
        "beta",
        NULL,
    };
    char *child_environment[] = {
        "LAB_MODE=execve",
        pid_environment,
        NULL,
    };

    if (argc > 1 && strcmp(argv[1], "alpha") == 0) {
        return child_mode(argc, argv);
    }

    if (snprintf(pid_text, sizeof(pid_text), "%ld", (long)getpid()) < 0 ||
        snprintf(pid_environment, sizeof(pid_environment), "ORIGINAL_PID=%s",
                 pid_text) < 0) {
        fprintf(stderr, "could not format PID\n");
        return EXIT_FAILURE;
    }

    execve("/proc/self/exe", child_argv, child_environment);
    perror("execve");
    return EXIT_FAILURE;
}
