/*
 * Experiment - Probe the permission required to raise CPU priority
 *
 * Safety:
 *   The request is isolated in a child. If the environment authorizes it, the
 *   elevated child exits immediately. The parent and interactive shell keep
 *   their original nice values.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

struct result {
    int before;
    int requested;
    int return_value;
    int error_number;
};

static int read_nice(void)
{
    int value;

    errno = 0;
    value = getpriority(PRIO_PROCESS, 0);
    return value == -1 && errno != 0 ? 1000 : value;
}

int main(void)
{
    int channel[2];
    struct result observation = {0, 0, -1, 0};
    int status;
    pid_t child;

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
        struct result local;

        (void)close(channel[0]);
        local.before = read_nice();
        if (local.before == 1000) {
            _exit(2);
        }
        local.requested = local.before > -20 ? local.before - 1 : -20;
        errno = 0;
        local.return_value =
            setpriority(PRIO_PROCESS, 0, local.requested);
        local.error_number = errno;
        if (write(channel[1], &local, sizeof(local)) !=
            (ssize_t)sizeof(local)) {
            _exit(3);
        }
        (void)close(channel[1]);
        _exit(0);
    }

    (void)close(channel[1]);
    if (read(channel[0], &observation, sizeof(observation)) !=
        (ssize_t)sizeof(observation)) {
        fputs("failed to read child result\n", stderr);
        return EXIT_FAILURE;
    }
    (void)close(channel[0]);
    if (waitpid(child, &status, 0) == -1 ||
        !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fputs("child failed\n", stderr);
        return EXIT_FAILURE;
    }

    printf("before=%d requested=%d raise_allowed=%s errno=%d\n",
           observation.before, observation.requested,
           observation.return_value == 0 ? "yes" : "no",
           observation.error_number);
    puts("probe_isolated_in_child=yes parent_priority_unchanged=yes");
    return EXIT_SUCCESS;
}
