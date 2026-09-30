/*
 * Exercise 04.06 - Drain an edge-triggered nonblocking descriptor
 *
 * Purpose:
 *   Demonstrate why EPOLLET users must keep reading until EAGAIN.
 *
 * Linux behavior:
 *   Edge-triggered epoll reports a transition into readiness. After one byte is
 *   read, the unread bytes do not create a new edge. The correct pattern uses a
 *   nonblocking descriptor and drains it until read() reports EAGAIN.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(void)
{
    char first_byte = '\0';
    char drain_buffer[8];
    int pipe_fds[2];
    int epoll_fd;
    int flags;
    struct epoll_event interest = {0};
    struct epoll_event event = {0};
    int first_wait;
    int second_wait;
    int close_failed = 0;
    size_t drained = 0;

    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    flags = fcntl(pipe_fds[0], F_GETFL);
    if (flags == -1 ||
        fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("make pipe reader nonblocking");
        goto failure;
    }

    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        goto failure;
    }

    interest.events = EPOLLIN | EPOLLET;
    interest.data.fd = pipe_fds[0];
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_fds[0], &interest) == -1 ||
        write(pipe_fds[1], "ABC", 3) != 3) {
        perror("prepare edge-triggered pipe");
        goto failure_with_epoll;
    }

    first_wait = epoll_wait(epoll_fd, &event, 1, 1000);
    if (first_wait != 1 || read(pipe_fds[0], &first_byte, 1) != 1) {
        fputs("first edge-triggered observation failed\n", stderr);
        goto failure_with_epoll;
    }

    /* BC is still unread, but no new transition has occurred. */
    second_wait = epoll_wait(epoll_fd, &event, 1, 0);
    if (second_wait != 0) {
        fputs("edge-triggered descriptor unexpectedly reported a new edge\n",
              stderr);
        goto failure_with_epoll;
    }

    for (;;) {
        ssize_t result = read(pipe_fds[0], drain_buffer, sizeof(drain_buffer));

        if (result > 0) {
            drained += (size_t) result;
        } else if (result == -1 && errno == EINTR) {
            continue;
        } else if (result == -1 && errno == EAGAIN) {
            break;
        } else {
            fputs("failed while draining edge-triggered pipe\n", stderr);
            goto failure_with_epoll;
        }
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
        perror("close edge-triggered resources");
        return EXIT_FAILURE;
    }

    printf("first_wait=%d second_wait=%d first=%c drained=%zu stopped=EAGAIN\n",
           first_wait, second_wait, first_byte, drained);
    return drained == 2 ? EXIT_SUCCESS : EXIT_FAILURE;

failure_with_epoll:
    (void) close(epoll_fd);
failure:
    (void) close(pipe_fds[0]);
    (void) close(pipe_fds[1]);
    return EXIT_FAILURE;
}
