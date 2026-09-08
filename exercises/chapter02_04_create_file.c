//
// Created by fonke on 8/20/2026.
//

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main (void){
    int fd;

    fd = open("love_demo.txt",
        O_WRONLY | O_CREAT | O_TRUNC,
        0664
    );

    if (fd == -1) {
        perror ("open");
        return 1;
    }

    printf("Created/opened file with fd=%d\n", fd);

    if (close(fd) == -1) {
        perror ("close");
        return 1;
    }

    return 0;
}