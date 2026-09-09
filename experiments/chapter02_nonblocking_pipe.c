#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    const char payload[] = "hello";
    char buffer[16];
    int pipe_fds[2];
    int flags;
    ssize_t result;

    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return 1;
    }

    flags = fcntl(pipe_fds[0], F_GETFL);
    if (flags == -1 || fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl O_NONBLOCK");
        (void) close(pipe_fds[0]);
        (void) close(pipe_fds[1]);
        return 1;
    }

    errno = 0;
    result = read(pipe_fds[0], buffer, sizeof(buffer));
    if (result == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        printf("before data: read returned -1, %s\n", strerror(errno));
    } else {
        fputs("unexpected first read result\n", stderr);
        (void) close(pipe_fds[0]);
        (void) close(pipe_fds[1]);
        return 1;
    }

    if (write(pipe_fds[1], payload, sizeof(payload) - 1) !=
        (ssize_t) (sizeof(payload) - 1)) {
        perror("write pipe");
        (void) close(pipe_fds[0]);
        (void) close(pipe_fds[1]);
        return 1;
    }

    result = read(pipe_fds[0], buffer, sizeof(buffer));
    if (result == -1) {
        perror("read pipe after write");
        (void) close(pipe_fds[0]);
        (void) close(pipe_fds[1]);
        return 1;
    }

    printf("after data: read returned %zd bytes: %.*s\n",
           result, (int) result, buffer);

    {
        int close_failed = 0;

        if (close(pipe_fds[0]) == -1) {
            perror("close pipe read end");
            close_failed = 1;
        }

        if (close(pipe_fds[1]) == -1) {
            perror("close pipe write end");
            close_failed = 1;
        }

        if (close_failed) {
            return 1;
        }
    }

    return 0;
}
