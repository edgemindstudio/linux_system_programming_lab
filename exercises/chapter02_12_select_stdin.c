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

    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    puts("waiting up to 5 seconds for standard input...");
    fflush(stdout);

    ready = select(STDIN_FILENO + 1, &read_set, NULL, NULL, &timeout);
    if (ready == -1) {
        if (errno == EINTR) {
            puts("select() was interrupted by a signal");
            return 0;
        }
        perror("select");
        return 1;
    }

    if (ready == 0) {
        puts("timeout: no input became ready");
        return 0;
    }

    if (FD_ISSET(STDIN_FILENO, &read_set)) {
        ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));

        if (bytes_read == -1) {
            perror("read stdin");
            return 1;
        }

        if (bytes_read == 0) {
            puts("stdin was ready because it reached EOF");
        } else {
            printf("stdin ready: read %zd bytes: %.*s",
                   bytes_read, (int) bytes_read, buffer);
            if (buffer[bytes_read - 1] != '\n') {
                putchar('\n');
            }
        }
    }

    return 0;
}
