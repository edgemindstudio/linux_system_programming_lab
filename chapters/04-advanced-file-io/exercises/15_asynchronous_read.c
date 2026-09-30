/*
 * Exercise 04.15 - Submit and collect a POSIX asynchronous read
 *
 * Purpose:
 *   Queue an input request, wait for completion, inspect its status, and collect
 *   its final byte count through the POSIX AIO control-block interface.
 *
 * Linux behavior:
 *   aio_read() reports whether submission succeeded. Completion is separate:
 *   aio_error() reports current status and aio_return() must be called exactly
 *   once after completion to obtain the operation's result.
 */

#define _POSIX_C_SOURCE 200809L

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INPUT_PATH "build/chapters/04-advanced-file-io/data/aio_read.txt"

int main(void)
{
    static const char text[] = "asynchronous-data";
    char buffer[sizeof(text)] = {0};
    struct aiocb request;
    const struct aiocb *wait_list[1];
    int fd = open(INPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    int completion_error;
    ssize_t result;

    if (fd == -1) {
        perror("open " INPUT_PATH);
        return EXIT_FAILURE;
    }

    if (pwrite(fd, text, sizeof(text) - 1, 0) !=
        (ssize_t) (sizeof(text) - 1)) {
        perror("prepare AIO input");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    memset(&request, 0, sizeof(request));
    request.aio_fildes = fd;
    request.aio_buf = buffer;
    request.aio_nbytes = sizeof(text) - 1;
    request.aio_offset = 0;
    request.aio_sigevent.sigev_notify = SIGEV_NONE;

    if (aio_read(&request) == -1) {
        perror("aio_read submission");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    wait_list[0] = &request;
    while ((completion_error = aio_error(&request)) == EINPROGRESS) {
        if (aio_suspend(wait_list, 1, NULL) == -1 && errno != EINTR) {
            perror("aio_suspend");
            (void) close(fd);
            return EXIT_FAILURE;
        }
    }

    if (completion_error != 0) {
        fprintf(stderr, "asynchronous read: %s\n",
                strerror(completion_error));
        (void) aio_return(&request);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    result = aio_return(&request);
    if (result != (ssize_t) (sizeof(text) - 1)) {
        fprintf(stderr, "expected %zu AIO bytes, received %zd\n",
                sizeof(text) - 1, result);
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1) {
        perror("close " INPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("submitted=yes completed=yes bytes=%zd text=%s\n", result, buffer);
    return EXIT_SUCCESS;
}
