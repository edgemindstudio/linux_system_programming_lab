//
// Created by fonke on 8/21/2026.
//

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    int fd1;
    int fd2;

    fd1 = creat("created_with_creat.txt", 0644);

    if (fd1 == -1) {
        perror("creat");
        return 1;
    }

    fd2 = open("created_with_open.txt",
        O_WRONLY | O_CREAT | O_TRUNC,
        0664
    );

    if (fd2 == -1)
    {
        perror("open");
        close(fd1);
        return 1;
    }

    printf("creat() fd = %d\n", fd1);
    printf("open() fd = %d\n", fd2);

    close(fd1);
    close(fd2);

    return 0;
}