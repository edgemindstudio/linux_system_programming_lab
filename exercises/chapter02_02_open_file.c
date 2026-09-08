//
// Created by fonke on 8/20/2026.
//

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    int fd;
    fd = open("/etc/hosts", O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    printf("open() returned file descriptor %d\n", fd);

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}