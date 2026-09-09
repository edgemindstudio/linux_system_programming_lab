#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CREAT_PATH "build/chapter02/data/created_with_creat.txt"
#define OPEN_PATH "build/chapter02/data/created_with_open.txt"
#define CREATION_MODE 0644

static int checked_close(int fd, const char *label)
{
    if (close(fd) == -1) {
        perror(label);
        return -1;
    }

    return 0;
}

int main(void)
{
    int creat_fd = creat(CREAT_PATH, CREATION_MODE);
    int open_fd;

    if (creat_fd == -1) {
        perror("creat " CREAT_PATH);
        return EXIT_FAILURE;
    }

    open_fd = open(OPEN_PATH,
                   O_WRONLY | O_CREAT | O_TRUNC,
                   CREATION_MODE);
    if (open_fd == -1) {
        perror("open " OPEN_PATH);
        (void) checked_close(creat_fd, "close creat descriptor");
        return EXIT_FAILURE;
    }

    printf("creat() fd = %d\n", creat_fd);
    printf("open() fd  = %d\n", open_fd);
    puts("Both files used the same requested mode and truncating write-only behavior.");

    if (checked_close(creat_fd, "close creat descriptor") == -1) {
        (void) checked_close(open_fd, "close open descriptor");
        return EXIT_FAILURE;
    }

    if (checked_close(open_fd, "close open descriptor") == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
