#define _XOPEN_SOURCE 700

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int write_all(int fd, const void *buffer, size_t count)
{
    const char *cursor = buffer;

    while (count > 0) {
        ssize_t written = write(fd, cursor, count);

        if (written > 0) {
            cursor += written;
            count -= (size_t) written;
        } else if (written == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }

    return 0;
}

int main(void)
{
    const char initial[] = "ABCDEFGHIJ";
    const char replacement[] = "xyz";
    char slice[4] = {0};
    char whole[sizeof(initial)] = {0};
    off_t before;
    off_t after_pread;
    off_t after_pwrite;
    int fd;

    fd = open("build/chapters/02-file-io/data/positional_demo.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open positional_demo.txt");
        return 1;
    }

    if (write_all(fd, initial, sizeof(initial) - 1) == -1 ||
        lseek(fd, 0, SEEK_SET) == (off_t) -1) {
        perror("initialize file");
        (void) close(fd);
        return 1;
    }

    before = lseek(fd, 0, SEEK_CUR);
    if (before == (off_t) -1 || pread(fd, slice, 3, 4) != 3) {
        perror("pread");
        (void) close(fd);
        return 1;
    }

    after_pread = lseek(fd, 0, SEEK_CUR);
    if (after_pread == (off_t) -1 ||
        pwrite(fd, replacement, sizeof(replacement) - 1, 1) !=
            (ssize_t) (sizeof(replacement) - 1)) {
        perror("pwrite");
        (void) close(fd);
        return 1;
    }

    after_pwrite = lseek(fd, 0, SEEK_CUR);
    if (after_pwrite == (off_t) -1) {
        perror("lseek after pwrite");
        (void) close(fd);
        return 1;
    }

    if (read(fd, whole, sizeof(initial) - 1) !=
        (ssize_t) (sizeof(initial) - 1)) {
        perror("ordinary read");
        (void) close(fd);
        return 1;
    }

    printf("pread(fd, 3 bytes, offset 4) -> \"%s\"\n", slice);
    printf("shared offset: before=%lld after pread=%lld after pwrite=%lld\n",
           (long long) before,
           (long long) after_pread,
           (long long) after_pwrite);
    printf("ordinary read from offset 0 -> \"%s\"\n", whole);

    if (close(fd) == -1) {
        perror("close positional_demo.txt");
        return 1;
    }

    return 0;
}
