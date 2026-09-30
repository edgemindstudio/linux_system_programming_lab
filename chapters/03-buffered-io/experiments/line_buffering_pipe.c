/*
 * Experiment — Newline-triggered flushing through a pipe
 *
 * Question:
 *   Does an explicitly line-buffered stream flush a partial line before a
 *   newline is written?
 *
 * Prediction:
 *   A nonblocking pipe reader sees EAGAIN while the partial line remains in the
 *   C-library buffer. Adding '\n' flushes the complete line into the pipe.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum { STREAM_BUFFER_SIZE = 128 };

int main(void)
{
    char stream_buffer[STREAM_BUFFER_SIZE];
    char observed[32] = {0};
    int pipe_fds[2];
    int flags;
    FILE *writer;
    ssize_t before_newline;
    ssize_t after_newline;
    int close_failed = 0;

    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    flags = fcntl(pipe_fds[0], F_GETFL);
    if (flags == -1 || fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("make pipe reader nonblocking");
        (void) close(pipe_fds[0]);
        (void) close(pipe_fds[1]);
        return EXIT_FAILURE;
    }

    writer = fdopen(pipe_fds[1], "w");
    if (writer == NULL) {
        perror("fdopen pipe writer");
        (void) close(pipe_fds[0]);
        (void) close(pipe_fds[1]);
        return EXIT_FAILURE;
    }

    if (setvbuf(writer, stream_buffer, _IOLBF, sizeof(stream_buffer)) != 0 ||
        fputs("partial", writer) == EOF) {
        fputs("prepare line-buffered writer failed\n", stderr);
        (void) close(pipe_fds[0]);
        (void) fclose(writer);
        return EXIT_FAILURE;
    }

    errno = 0;
    before_newline = read(pipe_fds[0], observed, sizeof(observed));
    if (before_newline != -1 || errno != EAGAIN) {
        fputs("expected EAGAIN before newline\n", stderr);
        (void) close(pipe_fds[0]);
        (void) fclose(writer);
        return EXIT_FAILURE;
    }

    if (fputc('\n', writer) == EOF) {
        perror("write newline");
        (void) close(pipe_fds[0]);
        (void) fclose(writer);
        return EXIT_FAILURE;
    }

    after_newline = read(pipe_fds[0], observed, sizeof(observed) - 1);
    if (after_newline != 8) {
        if (after_newline == -1) {
            perror("read flushed line");
        } else {
            fprintf(stderr,
                    "short pipe read: expected 8, received %zd\n",
                    after_newline);
        }
        (void) close(pipe_fds[0]);
        (void) fclose(writer);
        return EXIT_FAILURE;
    }

    if (close(pipe_fds[0]) == -1) {
        perror("close pipe reader");
        close_failed = 1;
    }
    if (fclose(writer) == EOF) {
        perror("fclose pipe writer");
        close_failed = 1;
    }
    if (close_failed) {
        return EXIT_FAILURE;
    }

    printf("before_newline=EAGAIN after_newline=%zd data=%s",
           after_newline,
           observed);
    return EXIT_SUCCESS;
}
