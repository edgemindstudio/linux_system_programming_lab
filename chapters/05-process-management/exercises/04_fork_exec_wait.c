/*
 * Exercise 05.04 — fork(), exec(), and waitpid() as one lifecycle
 *
 * Purpose:
 *   Launch a new program in a child, capture its standard output, and collect
 *   its termination status in the parent.
 *
 * Linux behavior:
 *   fork() creates the child process. dup2() connects that child's stdout to
 *   a pipe. execl() replaces the child's program image while preserving its
 *   PID and the pipe descriptor. A successful exec never returns.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int output_pipe[2];
    pid_t child;
    char output[128];
    size_t used = 0U;
    int status;

    if (pipe(output_pipe) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child == 0) {
        (void)close(output_pipe[0]);
        if (dup2(output_pipe[1], STDOUT_FILENO) == -1) {
            _exit(126);
        }
        (void)close(output_pipe[1]);
        execl("/bin/echo", "echo", "hello-from-exec", (char *)NULL);
        _exit(127);
    }

    (void)close(output_pipe[1]);
    while (used + 1U < sizeof(output)) {
        ssize_t count = read(output_pipe[0], output + used,
                             sizeof(output) - used - 1U);
        if (count == 0) {
            break;
        }
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            return EXIT_FAILURE;
        }
        used += (size_t)count;
    }
    output[used] = '\0';
    output[strcspn(output, "\n")] = '\0';
    (void)close(output_pipe[0]);

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("output=%s exited=%s status=%d\n",
           output,
           WIFEXITED(status) ? "yes" : "no",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);

    return EXIT_SUCCESS;
}
