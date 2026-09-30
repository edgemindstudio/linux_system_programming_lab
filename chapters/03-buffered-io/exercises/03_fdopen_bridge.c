/*
 * Exercise 03.03 — Wrap a descriptor in a FILE stream with fdopen()
 *
 * Purpose:
 *   Open a file using the Chapter 2 descriptor interface, transfer ownership of
 *   that descriptor to a standard-I/O stream, and verify the ownership rule.
 *
 * Linux behavior:
 *   fdopen() does not duplicate the descriptor. It constructs stream state
 *   around the same descriptor, and the mode must be compatible with the way
 *   the descriptor was opened. A successful fclose() flushes the stream and
 *   closes that underlying descriptor.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/fdopen_bridge.txt"

int main(void)
{
    int fd = open(OUTPUT_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FILE *stream;
    int stream_fd;

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    stream = fdopen(fd, "w");
    if (stream == NULL) {
        perror("fdopen");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    stream_fd = fileno(stream);
    if (stream_fd == -1) {
        perror("fileno");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fputs("descriptor now owned by a FILE stream\n", stream) == EOF) {
        perror("fputs");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose");
        return EXIT_FAILURE;
    }

    /* No intervening open occurs, so EBADF proves fclose() closed this fd. */
    errno = 0;
    if (fcntl(fd, F_GETFD) != -1 || errno != EBADF) {
        fputs("descriptor unexpectedly remained open\n", stderr);
        return EXIT_FAILURE;
    }

    printf("open fd=%d, stream fd=%d, closed_by_fclose=yes\n", fd, stream_fd);
    return EXIT_SUCCESS;
}
