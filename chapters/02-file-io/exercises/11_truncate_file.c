/*
 * Exercise 02.11 — Shrink and extend a file with ftruncate()
 *
 * Purpose:
 *   Change a file's logical length through an open descriptor and inspect what
 *   happens to the current offset, EOF, and newly exposed bytes.
 *
 * Linux behavior:
 *   Shrinking discards data beyond the new end-of-file. Extending a regular
 *   file makes the new region read as zero bytes. Neither operation changes the
 *   descriptor's current offset, even when that offset lies beyond the new EOF.
 */

#define _XOPEN_SOURCE 700

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

/* Transfer the entire initialization array despite possible short writes. */
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

    /* Write ten bytes, then deliberately position the descriptor at offset 8. */
    if (write_all(fd, initial, sizeof(initial) - 1) == -1 ||
        lseek(fd, 8, SEEK_SET) == (off_t) -1) {
        perror("initialize file");
        (void) close(fd);
        return 1;
    }

    /* Bytes at offsets 4 through 9 cease to be part of the file. */
    if (ftruncate(fd, 4) == -1) {
        perror("ftruncate smaller");
        (void) close(fd);
        return 1;
    }

    /* fstat() reports file metadata; lseek() proves the offset stayed at 8. */
    offset = lseek(fd, 0, SEEK_CUR);
    if (offset == (off_t) -1 || fstat(fd, &info) == -1) {
        perror("inspect smaller file");
        (void) close(fd);
        return 1;
    }

    printf("after shrinking: size=%lld, current offset=%lld\n",
           (long long) info.st_size, (long long) offset);

    /* A read starting beyond the new size returns zero immediately: EOF. */
    bytes_read = read(fd, bytes, sizeof(bytes));
    if (bytes_read == -1) {
        perror("read beyond new EOF");
        (void) close(fd);
        return 1;
    }
    printf("read at offset 8 after shrink returned %zd (EOF)\n", bytes_read);

    /* Extending creates a zero-filled logical region from offsets 4 through 11. */
    if (ftruncate(fd, 12) == -1) {
        perror("ftruncate larger");
        (void) close(fd);
        return 1;
    }

    /* Positional read verifies all 12 bytes without disturbing the offset. */
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
