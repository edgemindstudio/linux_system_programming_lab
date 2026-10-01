/*
 * Experiment — Build and verify a three-generation process tree
 *
 * Prediction:
 *   The first child reports the original process as its parent. The
 *   grandchild reports that first child as its parent.
 *
 * Observation strategy:
 *   Each descendant writes one small record to a pipe. The original parent
 *   prints the final relationships, avoiding scheduler-dependent output from
 *   multiple processes.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct identity {
    int generation;
    pid_t pid;
    pid_t parent_pid;
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
    pid_t original = getpid();
    int report_pipe[2];
    pid_t child;
    struct identity child_report;
    struct identity grandchild_report;
    int status;

    if (pipe(report_pipe) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    child = fork();
    if (child == -1) {
        perror("fork child");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        struct identity report = {
            .generation = 1,
            .pid = getpid(),
            .parent_pid = getppid(),
        };
        pid_t grandchild;
        int grandchild_status;

        (void)close(report_pipe[0]);
        if (exact_io(report_pipe[1], &report, sizeof(report), 1) == -1) {
            _exit(120);
        }
        grandchild = fork();
        if (grandchild == -1) {
            _exit(121);
        }
        if (grandchild == 0) {
            struct identity second_report = {
                .generation = 2,
                .pid = getpid(),
                .parent_pid = getppid(),
            };
            if (exact_io(report_pipe[1], &second_report,
                         sizeof(second_report), 1) == -1) {
                _exit(122);
            }
            _exit(0);
        }
        if (waitpid(grandchild, &grandchild_status, 0) == -1) {
            _exit(123);
        }
        _exit(WIFEXITED(grandchild_status) &&
                      WEXITSTATUS(grandchild_status) == 0
                  ? 0
                  : 124);
    }

    (void)close(report_pipe[1]);
    if (exact_io(report_pipe[0], &child_report,
                 sizeof(child_report), 0) == -1 ||
        exact_io(report_pipe[0], &grandchild_report,
                 sizeof(grandchild_report), 0) == -1) {
        perror("read process reports");
        return EXIT_FAILURE;
    }
    (void)close(report_pipe[0]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("child_parent_matches=%s grandchild_parent_matches=%s hierarchy_depth=2\n",
           child_report.generation == 1 && child_report.parent_pid == original
               ? "yes"
               : "no",
           grandchild_report.generation == 2 &&
                   grandchild_report.parent_pid == child_report.pid
               ? "yes"
               : "no");
    return EXIT_SUCCESS;
}
