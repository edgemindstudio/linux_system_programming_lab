//
// Created by fonke on 8/20/2026.
//

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main () {
    int read_fd;
    int write_fd;

    read_fd = open("/etc/hosts", O_RDONLY);

    if (read_fd == -1) {
        perror("open O_RDONLY");
    }
    else {
        printf("Opened /etc/hosts for reading: fd=%d\n", read_fd);
        close(read_fd);
    }

    write_fd = open("/etc/hosts", O_WRONLY);

    if (write_fd == -1) {
        perror("open O_WRONLY");
    }
    else {
        printf("Opened /etc/hosts for writing: fd=%d\n", write_fd);
        close(write_fd);
    }

    return 0;
}