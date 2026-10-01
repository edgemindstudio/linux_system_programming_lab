/*
 * Exercise 05.16 — Create a new session with setsid()
 *
 * Purpose:
 *   Let a forked child create a session and verify the defining relationships
 *   among its PID, process-group ID, and session ID.
 *
 * Linux behavior:
 *   setsid() fails for an existing process-group leader. A newly forked child
 *   is normally not one, so it can become both session leader and process-
 *   group leader, detached from any controlling terminal.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct session_report {
    pid_t pid;
    pid_t process_group;
    pid_t session;
};

static int transfer(int fd, void *buffer, size_t length, int writing)
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
    int report_pipe[2];
    pid_t child;
    struct session_report report;
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
        struct session_report child_report;
        pid_t session;

        (void)close(report_pipe[0]);
        session = setsid();
        if (session == (pid_t)-1) {
            _exit(120);
        }
        child_report.pid = getpid();
        child_report.process_group = getpgrp();
        child_report.session = session;
        if (transfer(report_pipe[1], &child_report,
                     sizeof(child_report), 1) == -1) {
            _exit(121);
        }
        _exit(0);
    }

    (void)close(report_pipe[1]);
    if (transfer(report_pipe[0], &report, sizeof(report), 0) == -1) {
        perror("read session report");
        return EXIT_FAILURE;
    }
    (void)close(report_pipe[0]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("pid_equals_pgid=%s pid_equals_sid=%s session_created=%s\n",
           report.pid == report.process_group ? "yes" : "no",
           report.pid == report.session ? "yes" : "no",
           report.session > 0 ? "yes" : "no");
    return EXIT_SUCCESS;
}
