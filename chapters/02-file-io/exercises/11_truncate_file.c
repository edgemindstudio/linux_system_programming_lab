#define _XOPEN_SOURCE 700

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
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
    const char initial[] = "0123456789";
    unsigned char bytes[12];
    struct stat info;
    off_t offset;
    ssize_t bytes_read;
    int fd;
    size_t index;

    fd = open("build/chapters/02-file-io/data/truncate_demo.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open truncate_demo.txt");
        return 1;
    }

    if (write_all(fd, initial, sizeof(initial) - 1) == -1 ||
        lseek(fd, 8, SEEK_SET) == (off_t) -1) {
        perror("initialize file");
        (void) close(fd);
        return 1;
    }

    if (ftruncate(fd, 4) == -1) {
        perror("ftruncate smaller");
        (void) close(fd);
        return 1;
    }

    offset = lseek(fd, 0, SEEK_CUR);
    if (offset == (off_t) -1 || fstat(fd, &info) == -1) {
        perror("inspect smaller file");
        (void) close(fd);
        return 1;
    }

    printf("after shrinking: size=%lld, current offset=%lld\n",
           (long long) info.st_size, (long long) offset);

    bytes_read = read(fd, bytes, sizeof(bytes));
    if (bytes_read == -1) {
        perror("read beyond new EOF");
        (void) close(fd);
        return 1;
    }
    printf("read at offset 8 after shrink returned %zd (EOF)\n", bytes_read);

    if (ftruncate(fd, 12) == -1) {
        perror("ftruncate larger");
        (void) close(fd);
        return 1;
    }

    bytes_read = pread(fd, bytes, sizeof(bytes), 0);
    if (bytes_read != (ssize_t) sizeof(bytes)) {
        perror("pread expanded file");
        (void) close(fd);
        return 1;
    }

    fputs("after extending to 12 bytes:", stdout);
    for (index = 0; index < sizeof(bytes); ++index) {
        printf(" %02x", bytes[index]);
    }
    putchar('\n');

    if (close(fd) == -1) {
        perror("close truncate_demo.txt");
        return 1;
    }

    return 0;
}
