/*
 * Exercise 05.18 — Inspect the safe core of daemon setup
 *
 * Purpose:
 *   Perform the important detachment steps in a short-lived child: create a
 *   new session, change to a known working directory, and redirect standard
 *   descriptors to /dev/null. A private report pipe remains open solely so the
 *   parent can verify the result.
 *
 * Linux behavior:
 *   Traditional daemons often add a second fork, a umask decision, descriptor
 *   closure, logging, a PID-file policy, and signal handling. Modern systemd
 *   services commonly remain in the foreground and let the service manager
 *   own that lifecycle instead of self-daemonizing.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct daemon_report {
    pid_t pid;
    pid_t process_group;
    pid_t session;
    char working_directory[8];
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
    struct daemon_report report;
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
        struct daemon_report child_report;
        int null_fd;

        (void)close(report_pipe[0]);
        if (setsid() == (pid_t)-1 || chdir("/") == -1) {
            _exit(120);
        }
        null_fd = open("/dev/null", O_RDWR);
        if (null_fd == -1 ||
            dup2(null_fd, STDIN_FILENO) == -1 ||
            dup2(null_fd, STDOUT_FILENO) == -1 ||
            dup2(null_fd, STDERR_FILENO) == -1) {
            _exit(121);
        }
        if (null_fd > STDERR_FILENO) {
            (void)close(null_fd);
        }

        child_report.pid = getpid();
        child_report.process_group = getpgrp();
        child_report.session = getsid(0);
        if (getcwd(child_report.working_directory,
                   sizeof(child_report.working_directory)) == NULL) {
            _exit(122);
        }
        if (transfer(report_pipe[1], &child_report,
                     sizeof(child_report), 1) == -1) {
            _exit(123);
        }
        (void)close(report_pipe[1]);
        _exit(0);
    }

    (void)close(report_pipe[1]);
    if (transfer(report_pipe[0], &report, sizeof(report), 0) == -1) {
        perror("read daemon report");
        return EXIT_FAILURE;
    }
    (void)close(report_pipe[0]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("session_leader=%s process_group_leader=%s cwd_root=%s\n",
           report.session == report.pid ? "yes" : "no",
           report.process_group == report.pid ? "yes" : "no",
           strcmp(report.working_directory, "/") == 0 ? "yes" : "no");
    printf("standard_streams_redirected=yes child_exit=%d\n",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    return EXIT_SUCCESS;
}
