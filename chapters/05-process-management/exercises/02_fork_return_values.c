/*
 * Exercise 05.02 — The two return paths from fork()
 *
 * Purpose:
 *   Show that one successful fork() call produces two executing processes.
 *   The parent receives the child's positive PID; the child receives zero.
 *
 * Linux behavior:
 *   Parent and child resume after the same fork() call. Scheduling order is
 *   deliberately unspecified, so this program sends the child's observations
 *   through a pipe and lets the parent print one deterministic summary.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct child_report {
    pid_t fork_result;
    pid_t pid;
    pid_t parent_pid;
};

static int write_all(int fd, const void *buffer, size_t length)
{
    const unsigned char *cursor = buffer;

    while (length > 0U) {
        ssize_t written = write(fd, cursor, length);
        if (written == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        cursor += (size_t)written;
        length -= (size_t)written;
    }
    return 0;
}

static int read_all(int fd, void *buffer, size_t length)
{
    unsigned char *cursor = buffer;

    while (length > 0U) {
        ssize_t received = read(fd, cursor, length);
        if (received == 0) {
            return -1;
        }
        if (received == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        cursor += (size_t)received;
        length -= (size_t)received;
    }
    return 0;
}

int main(void)
{
    int report_pipe[2];
    pid_t child;
    struct child_report report;
    int status;

    if (pipe(report_pipe) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child == 0) {
        struct child_report child_view = {
            .fork_result = child,
            .pid = getpid(),
            .parent_pid = getppid(),
        };

        (void)close(report_pipe[0]);
        if (write_all(report_pipe[1], &child_view, sizeof(child_view)) == -1) {
            _exit(2);
        }
        (void)close(report_pipe[1]);
        _exit(0);
    }

    (void)close(report_pipe[1]);
    if (read_all(report_pipe[0], &report, sizeof(report)) == -1) {
        perror("read child report");
        return EXIT_FAILURE;
    }
    (void)close(report_pipe[0]);

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("parent_fork_result=%ld child_fork_result=%ld\n",
           (long)child,
           (long)report.fork_result);
    printf("child_pid_matches=%s child_parent_matches=%s child_exit=%d\n",
           report.pid == child ? "yes" : "no",
           report.parent_pid == getpid() ? "yes" : "no",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);

    return EXIT_SUCCESS;
}
