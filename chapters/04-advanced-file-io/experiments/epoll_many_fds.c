/*
 * Experiment - A small ready set inside a larger epoll interest set
 *
 * Question:
 *   When many descriptors are registered, does epoll_wait() return the entire
 *   interest set or only descriptors with pending events?
 *
 * Prediction:
 *   Sixty-four pipe readers are registered, but only three are made readable;
 *   epoll_wait() should return exactly those three ready entries.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

enum { PIPE_COUNT = 64, READY_COUNT = 3 };

int main(void)
{
    const int ready_indexes[READY_COUNT] = {3, 17, 42};
    int pipes[PIPE_COUNT][2];
    int epoll_fd = -1;
    struct epoll_event events[PIPE_COUNT];
    int ready_seen[PIPE_COUNT] = {0};
    int created = 0;
    int index;
    int ready;
    int status = EXIT_FAILURE;

    for (index = 0; index < PIPE_COUNT; ++index) {
        pipes[index][0] = -1;
        pipes[index][1] = -1;
    }

    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        goto cleanup;
    }

    for (index = 0; index < PIPE_COUNT; ++index) {
        struct epoll_event interest = {0};

        if (pipe(pipes[index]) == -1) {
            perror("pipe");
            goto cleanup;
        }
        ++created;
        interest.events = EPOLLIN;
        interest.data.u32 = (unsigned int) index;
        if (epoll_ctl(epoll_fd,
                      EPOLL_CTL_ADD,
                      pipes[index][0],
                      &interest) == -1) {
            perror("epoll_ctl ADD");
            goto cleanup;
        }
    }

    for (index = 0; index < READY_COUNT; ++index) {
        int selected = ready_indexes[index];

        if (write(pipes[selected][1], "X", 1) != 1) {
            perror("write ready pipe");
            goto cleanup;
        }
    }

    ready = epoll_wait(epoll_fd, events, PIPE_COUNT, 1000);
    if (ready != READY_COUNT) {
        fprintf(stderr, "expected %d ready descriptors, received %d\n",
                READY_COUNT, ready);
        goto cleanup;
    }

    for (index = 0; index < ready; ++index) {
        unsigned int selected = events[index].data.u32;

        if (selected >= PIPE_COUNT) {
            fputs("epoll returned an invalid user-data index\n", stderr);
            goto cleanup;
        }
        ready_seen[selected] = 1;
    }

    for (index = 0; index < READY_COUNT; ++index) {
        if (!ready_seen[ready_indexes[index]]) {
            fputs("epoll omitted an expected ready descriptor\n", stderr);
            goto cleanup;
        }
    }

    printf("registered=%d ready=%d returned_only_ready=yes\n",
           PIPE_COUNT, ready);
    status = EXIT_SUCCESS;

cleanup:
    for (index = 0; index < created; ++index) {
        (void) close(pipes[index][0]);
        (void) close(pipes[index][1]);
    }
    if (epoll_fd != -1) {
        (void) close(epoll_fd);
    }
    return status;
}
