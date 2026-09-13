#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/02-file-io/data/love_demo.txt"

int main(void)
{
    int fd = open(OUTPUT_PATH,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0664);

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("Created or opened %s with fd=%d\n", OUTPUT_PATH, fd);

    if (close(fd) == -1) {
        perror("close " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
