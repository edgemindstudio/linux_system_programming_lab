#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum { ALIGNMENT = 4096, BLOCK_SIZE = 4096 };

int main(void)
{
    void *buffer = NULL;
    int fd;
    ssize_t written;

    if (posix_memalign(&buffer, ALIGNMENT, BLOCK_SIZE) != 0) {
        fputs("posix_memalign failed\n", stderr);
        return 1;
    }

    memset(buffer, 'D', BLOCK_SIZE);

    fd = open("build/chapter02/data/direct_io_demo.bin",
              O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT,
              0644);
    if (fd == -1) {
        fprintf(stderr,
                "O_DIRECT is unsupported by this filesystem or open failed: %s\n",
                strerror(errno));
        free(buffer);
        return 2;
    }

    written = write(fd, buffer, BLOCK_SIZE);
    if (written == -1) {
        fprintf(stderr,
                "aligned O_DIRECT write failed: %s\n",
                strerror(errno));
        (void) close(fd);
        free(buffer);
        return 2;
    }

    printf("O_DIRECT write returned %zd bytes using a %d-byte aligned buffer.\n",
           written, ALIGNMENT);

    if (close(fd) == -1) {
        perror("close direct_io_demo.bin");
        free(buffer);
        return 1;
    }

    free(buffer);
    return written == BLOCK_SIZE ? 0 : 1;
}
