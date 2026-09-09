#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int read_fd = open("/etc/hosts", O_RDONLY);
    int write_fd;

    if (read_fd == -1) {
        perror("open /etc/hosts with O_RDONLY");
        return EXIT_FAILURE;
    }

    printf("Opened /etc/hosts for reading: fd=%d\n", read_fd);

    if (close(read_fd) == -1) {
        perror("close read descriptor");
        return EXIT_FAILURE;
    }

    write_fd = open("/etc/hosts", O_WRONLY);
    if (write_fd == -1) {
        perror("O_WRONLY was rejected");
        puts("This is expected for an ordinary user without write permission.");
        return EXIT_SUCCESS;
    }

    printf("O_WRONLY unexpectedly succeeded: fd=%d\n", write_fd);
    puts("No write was attempted. Check the account and file permissions.");

    if (close(write_fd) == -1) {
        perror("close write descriptor");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
