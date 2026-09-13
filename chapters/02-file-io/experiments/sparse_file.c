#define _XOPEN_SOURCE 700

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

enum { HOLE_OFFSET = 1024 * 1024 };

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
    const char beginning[] = "BEGIN";
    const char ending[] = "END";
    unsigned char hole_bytes[8];
    struct stat info;
    int fd;
    size_t index;

    fd = open("build/chapters/02-file-io/data/sparse_demo.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open sparse_demo.bin");
        return 1;
    }

    if (write_all(fd, beginning, sizeof(beginning) - 1) == -1) {
        perror("write beginning");
        (void) close(fd);
        return 1;
    }

    if (lseek(fd, HOLE_OFFSET, SEEK_SET) == (off_t) -1) {
        perror("lseek past end");
        (void) close(fd);
        return 1;
    }

    if (write_all(fd, ending, sizeof(ending) - 1) == -1) {
        perror("write ending");
        (void) close(fd);
        return 1;
    }

    if (fstat(fd, &info) == -1) {
        perror("fstat sparse_demo.bin");
        (void) close(fd);
        return 1;
    }

    if (pread(fd, hole_bytes, sizeof(hole_bytes), 4096) !=
        (ssize_t) sizeof(hole_bytes)) {
        perror("pread hole");
        (void) close(fd);
        return 1;
    }

    printf("logical size: %lld bytes\n", (long long) info.st_size);
    printf("allocated space reported by st_blocks: %lld bytes\n",
           (long long) info.st_blocks * 512LL);
    fputs("bytes read from the hole:", stdout);
    for (index = 0; index < sizeof(hole_bytes); ++index) {
        printf(" %02x", hole_bytes[index]);
    }
    putchar('\n');

    if (close(fd) == -1) {
        perror("close sparse_demo.bin");
        return 1;
    }

    return 0;
}
