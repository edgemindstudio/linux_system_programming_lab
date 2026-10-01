/*
 * Exercise 05.15 — Place a child in a new process group
 *
 * Purpose:
 *   Make one child the leader of a new process group while leaving the parent
 *   in its original group.
 *
 * Linux behavior:
 *   setpgid(child, child) assigns a process-group ID equal to the child's PID.
 *   Shells use process groups to address an entire pipeline as one job. A pipe
 *   gate prevents races while the parent performs the group change.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct group_report {
    pid_t pid;
    pid_t process_group;
};

static int exact_io(int fd, void *buffer, size_t length, int writing)
{
    unsigned char *cursor = buffer;

    while (length > 0U) {
        ssize_t count = writing != 0
                            ? write(fd, cursor, length)
                            : read(fd, cursor, length);
        if (count == 0 && writing == 0) {
            return -1;
        }
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        cursor += (size_t)count;
        length -= (size_t)count;
    }
    return 0;
}

int main(void)
{
    pid_t parent_group = getpgrp();
    int gate[2];
    int report_pipe[2];
    pid_t child;
    struct group_report report;
    int status;
    char token = 'G';

    if (pipe(gate) == -1 || pipe(report_pipe) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        char received;
        struct group_report child_report;

        (void)close(gate[1]);
        (void)close(report_pipe[0]);
        if (read(gate[0], &received, 1U) != 1) {
            _exit(120);
        }
        child_report.pid = getpid();
        child_report.process_group = getpgrp();
        if (exact_io(report_pipe[1], &child_report,
                     sizeof(child_report), 1) == -1) {
            _exit(121);
        }
        _exit(0);
    }

    (void)close(gate[0]);
    (void)close(report_pipe[1]);
    if (setpgid(child, child) == -1) {
        perror("setpgid");
        return EXIT_FAILURE;
    }
    if (write(gate[1], &token, 1U) != 1) {
        perror("write gate");
        return EXIT_FAILURE;
    }
    (void)close(gate[1]);
    if (exact_io(report_pipe[0], &report, sizeof(report), 0) == -1) {
        perror("read report");
        return EXIT_FAILURE;
    }
    (void)close(report_pipe[0]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("child_group_equals_child=%s parent_group_unchanged=%s\n",
           report.process_group == report.pid ? "yes" : "no",
           getpgrp() == parent_group ? "yes" : "no");
    return WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
