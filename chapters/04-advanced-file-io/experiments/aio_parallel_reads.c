/*
 * Experiment - Multiple POSIX AIO requests in flight
 *
 * Question:
 *   Can several independent file regions be submitted before any result is
 *   collected?
 *
 * Prediction:
 *   Three requests are queued first, aio_suspend() waits while any are pending,
 *   and each result is collected exactly once with aio_return().
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

#define INPUT_PATH "build/chapters/04-advanced-file-io/data/aio_parallel.txt"

enum { REQUEST_COUNT = 3, FIELD_SIZE = 5 };

int main(void)
{
    static const char encoded[] = "alpha" "bravo" "gamma";
    char buffers[REQUEST_COUNT][FIELD_SIZE + 1] = {{0}};
    struct aiocb requests[REQUEST_COUNT];
    const struct aiocb *wait_list[REQUEST_COUNT];
    int fd = open(INPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    int index;
    int pending;

    if (fd == -1) {
        perror("open " INPUT_PATH);
        return EXIT_FAILURE;
    }
    if (pwrite(fd, encoded, sizeof(encoded) - 1, 0) !=
        (ssize_t) (sizeof(encoded) - 1)) {
        perror("prepare parallel AIO input");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    memset(requests, 0, sizeof(requests));
    for (index = 0; index < REQUEST_COUNT; ++index) {
        requests[index].aio_fildes = fd;
        requests[index].aio_buf = buffers[index];
        requests[index].aio_nbytes = FIELD_SIZE;
        requests[index].aio_offset = (off_t) index * FIELD_SIZE;
        requests[index].aio_sigevent.sigev_notify = SIGEV_NONE;
        wait_list[index] = &requests[index];

        if (aio_read(&requests[index]) == -1) {
            perror("aio_read parallel submission");
            (void) close(fd);
            return EXIT_FAILURE;
        }
    }

    do {
        pending = 0;
        for (index = 0; index < REQUEST_COUNT; ++index) {
            if (aio_error(&requests[index]) == EINPROGRESS) {
                ++pending;
            }
        }

        if (pending > 0 &&
            aio_suspend(wait_list, REQUEST_COUNT, NULL) == -1 &&
            errno != EINTR) {
            perror("aio_suspend parallel requests");
            (void) close(fd);
            return EXIT_FAILURE;
        }
    } while (pending > 0);

    for (index = 0; index < REQUEST_COUNT; ++index) {
        int completion_error = aio_error(&requests[index]);
        ssize_t result;

        if (completion_error != 0) {
            fprintf(stderr, "parallel request %d: %s\n",
                    index, strerror(completion_error));
            (void) close(fd);
            return EXIT_FAILURE;
        }
        result = aio_return(&requests[index]);
        if (result != FIELD_SIZE) {
            fprintf(stderr, "parallel request %d returned %zd bytes\n",
                    index, result);
            (void) close(fd);
            return EXIT_FAILURE;
        }
    }

    if (close(fd) == -1) {
        perror("close " INPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("requests=%d completed=%d data=%s,%s,%s\n",
           REQUEST_COUNT,
           REQUEST_COUNT,
           buffers[0], buffers[1], buffers[2]);
    return EXIT_SUCCESS;
}
