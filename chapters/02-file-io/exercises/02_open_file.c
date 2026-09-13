#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int fd = open("/etc/hosts", O_RDONLY);

    if (fd == -1) {
        perror("open /etc/hosts");
        return EXIT_FAILURE;
    }

    printf("open() returned file descriptor %d\n", fd);

    if (close(fd) == -1) {
        perror("close /etc/hosts");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
