/*
 * Exercise 06.05 - Nice values cross fork()
 *
 * Purpose:
 *   Show that a child begins with the scheduling nice value of its parent.
 *
 * Linux behavior:
 *   fork() creates a new schedulable task. The child receives its own PID and
 *   accounting state, but it inherits the parent's nice value. A pipe carries
 *   the child's observation back without relying on output ordering.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

static int current_nice(void)
{
    int value;

    errno = 0;
    value = getpriority(PRIO_PROCESS, 0);
    if (value == -1 && errno != 0) {
        return 1000;
    }
    return value;
}

int main(void)
{
    int channel[2];
    int parent_nice;
    int child_nice = 1000;
    int status;
    pid_t child;
    ssize_t transferred;

    parent_nice = current_nice();
    if (parent_nice == 1000) {
        perror("getpriority parent");
        return EXIT_FAILURE;
    }

    if (pipe(channel) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child == 0) {
        int value = current_nice();

        (void)close(channel[0]);
        transferred = write(channel[1], &value, sizeof(value));
        (void)close(channel[1]);
        _exit(transferred == (ssize_t)sizeof(value) ? 0 : 2);
    }

    (void)close(channel[1]);
    transferred = read(channel[0], &child_nice, sizeof(child_nice));
    (void)close(channel[0]);

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (transferred != (ssize_t)sizeof(child_nice) ||
        !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fputs("failed to collect child priority\n", stderr);
        return EXIT_FAILURE;
    }

    printf("parent_nice=%d child_nice=%d\n", parent_nice, child_nice);
    printf("child_inherited=yes values_match=%s\n",
           child_nice == parent_nice ? "yes" : "no");

    return EXIT_SUCCESS;
}
