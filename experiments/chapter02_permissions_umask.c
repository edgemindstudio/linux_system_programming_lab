#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static void print_mode(const char *label, mode_t mode)
{
    printf("%s %04o\n", label, (unsigned int) (mode & 0777));
}

int main(void)
{
    const char *path = "build/chapter02/data/permissions_demo.txt";
    mode_t old_umask;
    struct stat info;
    int fd;

    if (unlink(path) == -1 && errno != ENOENT) {
        perror("unlink old demo file");
        return 1;
    }

    old_umask = umask(0022);

    fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd == -1) {
        perror("open create");
        (void) umask(old_umask);
        return 1;
    }

    if (fstat(fd, &info) == -1) {
        perror("fstat new file");
        (void) close(fd);
        (void) umask(old_umask);
        return 1;
    }

    print_mode("requested mode:", 0666);
    print_mode("process umask: ", 0022);
    print_mode("actual mode:   ", info.st_mode);

    if (close(fd) == -1) {
        perror("close new file");
        (void) umask(old_umask);
        return 1;
    }

    fd = open(path, O_WRONLY | O_CREAT, 0600);
    if (fd == -1) {
        perror("reopen existing file");
        (void) umask(old_umask);
        return 1;
    }

    if (fstat(fd, &info) == -1) {
        perror("fstat existing file");
        (void) close(fd);
        (void) umask(old_umask);
        return 1;
    }

    print_mode("mode after reopening with 0600:", info.st_mode);
    puts("The mode argument is used only when open() actually creates the file.");

    if (close(fd) == -1) {
        perror("close existing file");
        (void) umask(old_umask);
        return 1;
    }

    (void) umask(old_umask);
    return 0;
}
