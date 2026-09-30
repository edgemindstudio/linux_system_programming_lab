/*
 * Exercise 02.12 — Wait for standard input with select()
 *
 * Purpose:
 *   Block for at most five seconds until standard input becomes readable, then
 *   distinguish available data, EOF, timeout, interruption, and failure.
 *
 * Linux behavior:
 *   Readiness means a read() can proceed without blocking; it does not promise
 *   payload bytes. A descriptor at EOF is also readable because read() can
 *   return zero immediately. select() modifies the descriptor sets and timeout
 *   passed to it, so they must be rebuilt before each repeated call.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

enum { BUFFER_SIZE = 256 };

int main(void)
{
    char buffer[BUFFER_SIZE];
    fd_set read_set;
    struct timeval timeout;
    int ready;

    /* Begin with an empty set, then ask select() to monitor descriptor 0. */
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);

    /* timeval expresses the maximum blocking interval in seconds/microseconds. */
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    puts("waiting up to 5 seconds for standard input...");
    fflush(stdout);

    /* The first argument is one greater than the highest descriptor examined. */
    ready = select(STDIN_FILENO + 1, &read_set, NULL, NULL, &timeout);
    if (ready == -1) {
        if (errno == EINTR) {
            /* A caught signal interrupted the wait before readiness or timeout. */
            puts("select() was interrupted by a signal");
            return 0;
        }
        perror("select");
        return 1;
    }

    if (ready == 0) {
        /* Zero means the timeout expired and no monitored descriptor was ready. */
        puts("timeout: no input became ready");
        return 0;
    }

    /* select() leaves only ready descriptors marked in read_set. */
    if (FD_ISSET(STDIN_FILENO, &read_set)) {
        ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));

        if (bytes_read == -1) {
            perror("read stdin");
            return 1;
        }

        if (bytes_read == 0) {
            puts("stdin was ready because it reached EOF");
        } else {
            /* %.*s limits printf() to the exact byte count returned by read(). */
            printf("stdin ready: read %zd bytes: %.*s",
                   bytes_read, (int) bytes_read, buffer);
            if (buffer[bytes_read - 1] != '\n') {
                putchar('\n');
            }
        }
    }

    return 0;
}
