/*
 * Exercise 04.05 - Level-triggered readiness remains while data remains
 *
 * Purpose:
 *   Read only one byte from a ready pipe and ask epoll about it again.
 *
 * Linux behavior:
 *   Level-triggered mode is the default. As long as unread bytes keep the pipe
 *   readable, epoll_wait() continues to report EPOLLIN on later calls.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(void)
{
    char first_byte = '\0';
    char remaining[3] = {0};
    int pipe_fds[2];
    int epoll_fd;
    struct epoll_event interest = {0};
    struct epoll_event event = {0};
    int first_wait;
    int second_wait;
    int close_failed = 0;

    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        goto failure;
    }

    interest.events = EPOLLIN;
    interest.data.fd = pipe_fds[0];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_fds[0], &interest) == -1 ||
        write(pipe_fds[1], "ABC", 3) != 3) {
        perror("prepare level-triggered pipe");
        goto failure_with_epoll;
    }

    first_wait = epoll_wait(epoll_fd, &event, 1, 1000);
    if (first_wait != 1 || read(pipe_fds[0], &first_byte, 1) != 1) {
        fputs("first level-triggered observation failed\n", stderr);
        goto failure_with_epoll;
    }

    second_wait = epoll_wait(epoll_fd, &event, 1, 0);
    if (second_wait != 1 || read(pipe_fds[0], remaining, 2) != 2) {
        fputs("second level-triggered observation failed\n", stderr);
        goto failure_with_epoll;
    }

    if (close(pipe_fds[0]) == -1) {
        close_failed = 1;
    }
    if (close(pipe_fds[1]) == -1) {
        close_failed = 1;
    }
    if (close(epoll_fd) == -1) {
        close_failed = 1;
    }
    if (close_failed != 0) {
        perror("close level-triggered resources");
        return EXIT_FAILURE;
    }

    printf("first_wait=%d second_wait=%d first=%c remaining=%s\n",
           first_wait, second_wait, first_byte, remaining);
    return EXIT_SUCCESS;

failure_with_epoll:
    (void) close(epoll_fd);
failure:
    (void) close(pipe_fds[0]);
    (void) close(pipe_fds[1]);
    return EXIT_FAILURE;
}
