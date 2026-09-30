/*
 * Exercise 04.03 - Register, wait for, modify, and remove an epoll watch
 *
 * Purpose:
 *   Follow the complete epoll lifecycle with the readable end of a pipe.
 *
 * Linux behavior:
 *   epoll_create1() returns a descriptor for an in-kernel interest set.
 *   epoll_ctl() changes that set, while epoll_wait() returns only descriptors
 *   whose requested readiness state is currently satisfied.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

static int close_checked(int fd, const char *label)
{
    if (close(fd) == -1) {
        perror(label);
        return -1;
    }
    return 0;
}

int main(void)
{
    static const char message[] = "ready";
    char buffer[sizeof(message)] = {0};
    int pipe_fds[2];
    int epoll_fd;
    struct epoll_event interest = {0};
    struct epoll_event ready = {0};
    int event_count;
    ssize_t bytes_read;
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
        goto failure;
    }

    if (write(pipe_fds[1], message, sizeof(message) - 1) !=
        (ssize_t) (sizeof(message) - 1)) {
        perror("write pipe message");
        goto failure;
    }

    do {
        event_count = epoll_wait(epoll_fd, &ready, 1, 1000);
    } while (event_count == -1 && errno == EINTR);

    if (event_count != 1 || ready.data.fd != pipe_fds[0] ||
        (ready.events & EPOLLIN) == 0U) {
        fputs("epoll did not report the expected readable pipe\n", stderr);
        goto failure;
    }

    bytes_read = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    if (bytes_read != (ssize_t) (sizeof(message) - 1)) {
        perror("read pipe message");
        goto failure;
    }

    interest.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, pipe_fds[0], &interest) == -1 ||
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_fds[0], NULL) == -1) {
        perror("epoll_ctl MOD or DEL");
        goto failure;
    }

    close_failed |= close_checked(pipe_fds[0], "close pipe reader") == -1;
    close_failed |= close_checked(pipe_fds[1], "close pipe writer") == -1;
    close_failed |= close_checked(epoll_fd, "close epoll descriptor") == -1;
    if (close_failed) {
        return EXIT_FAILURE;
    }

    printf("events=%d readable=yes message=%s lifecycle=add-mod-del\n",
           event_count, buffer);
    return EXIT_SUCCESS;

failure:
    (void) close(pipe_fds[0]);
    (void) close(pipe_fds[1]);
    (void) close(epoll_fd);
    return EXIT_FAILURE;
}
