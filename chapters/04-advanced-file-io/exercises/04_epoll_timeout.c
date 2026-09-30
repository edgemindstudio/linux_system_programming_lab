/*
 * Exercise 04.04 - Observe an epoll timeout
 *
 * Purpose:
 *   Wait on a registered descriptor when no event will become ready.
 *
 * Linux behavior:
 *   epoll_wait() returns 0 when the timeout expires. A return of 0 is not an
 *   error and does not set errno; it simply means no watched descriptor became
 *   ready during the requested interval.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(void)
{
    int pipe_fds[2];
    int epoll_fd;
    struct epoll_event interest = {0};
    struct epoll_event event = {0};
    int result;
    int close_failed = 0;

    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        (void) close(pipe_fds[0]);
        (void) close(pipe_fds[1]);
        return EXIT_FAILURE;
    }

    interest.events = EPOLLIN;
    interest.data.fd = pipe_fds[0];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_fds[0], &interest) == -1) {
        perror("epoll_ctl ADD");
        (void) close(pipe_fds[0]);
        (void) close(pipe_fds[1]);
        (void) close(epoll_fd);
        return EXIT_FAILURE;
    }

    do {
        result = epoll_wait(epoll_fd, &event, 1, 25);
    } while (result == -1 && errno == EINTR);

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
        perror("close epoll timeout resources");
        return EXIT_FAILURE;
    }

    if (result != 0) {
        fprintf(stderr, "expected timeout result 0, received %d\n", result);
        return EXIT_FAILURE;
    }

    printf("timeout_ms=25 ready_events=%d outcome=timeout\n", result);
    return EXIT_SUCCESS;
}
