/*
 * Exercise 05.17 — Launch untrusted text without a shell
 *
 * Purpose:
 *   Pass text containing shell metacharacters directly as one argument to an
 *   executable. The semicolon remains data because no shell parses it.
 *
 * Linux behavior:
 *   exec functions with an explicit path do not interpret redirection,
 *   variables, pipelines, or separators. This is safer than assembling a
 *   command string for system() when any portion comes from an untrusted user.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    const char *argument = argc > 1 ? argv[1] : "hello; echo injected";
    int output_pipe[2];
    pid_t child;
    char output[256];
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
        execl("/usr/bin/printf", "printf", "%s\n", argument, (char *)NULL);
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

    printf("output=%s shell_interpreted=no status=%d\n",
           output,
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    return EXIT_SUCCESS;
}
