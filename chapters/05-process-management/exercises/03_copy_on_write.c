/*
 * Exercise 05.03 — Copy-on-write after fork()
 *
 * Purpose:
 *   Demonstrate that parent and child begin with the same virtual-memory
 *   contents, yet a write in the child does not change the parent's variable.
 *
 * Linux behavior:
 *   fork() initially lets both page tables refer to the same physical pages.
 *   The pages are protected so the kernel can copy a page when one process
 *   first writes it. Equal virtual addresses therefore do not imply one
 *   shared C object after fork().
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct child_report {
    int value_before;
    int value_after;
    uintptr_t virtual_address;
};

static int transfer_exact(int fd, void *buffer, size_t length, int writing)
{
    unsigned char *cursor = buffer;

    while (length > 0U) {
        ssize_t result = writing != 0
                             ? write(fd, cursor, length)
                             : read(fd, cursor, length);
        if (result == 0 && writing == 0) {
            return -1;
        }
        if (result == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        cursor += (size_t)result;
        length -= (size_t)result;
    }
    return 0;
}

int main(void)
{
    int value = 41;
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
        struct child_report child_view;

        (void)close(report_pipe[0]);
        child_view.value_before = value;
        child_view.virtual_address = (uintptr_t)&value;
        value = 99;
        child_view.value_after = value;
        if (transfer_exact(report_pipe[1], &child_view,
                           sizeof(child_view), 1) == -1) {
            _exit(2);
        }
        (void)close(report_pipe[1]);
        _exit(0);
    }

    (void)close(report_pipe[1]);
    if (transfer_exact(report_pipe[0], &report, sizeof(report), 0) == -1) {
        perror("read child report");
        return EXIT_FAILURE;
    }
    (void)close(report_pipe[0]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("parent_value=%d child_before=%d child_after=%d\n",
           value,
           report.value_before,
           report.value_after);
    printf("same_virtual_address=%s independent_values=%s\n",
           report.virtual_address == (uintptr_t)&value ? "yes" : "no",
           value == 41 && report.value_after == 99 ? "yes" : "no");

    return WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
