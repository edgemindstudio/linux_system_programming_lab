/*
 * Exercise 02.10 — Perform positional I/O with pread() and pwrite()
 *
 * Purpose:
 *   Read and write at explicit byte positions while proving that positional
 *   I/O leaves the open-file description's shared current offset unchanged.
 *
 * Linux behavior:
 *   pread() and pwrite() combine positioning and transfer in one operation.
 *   Unlike an lseek() followed by read() or write(), another thread cannot
 *   change the targeted position between those two conceptual steps.
 */

#define _XOPEN_SOURCE 700

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

/* Ordinary write() still requires a loop because it may make partial progress. */
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

    /* Initialize ten bytes, then establish current offset zero for comparison. */
    if (write_all(fd, initial, sizeof(initial) - 1) == -1 ||
        lseek(fd, 0, SEEK_SET) == (off_t) -1) {
        perror("initialize file");
        (void) close(fd);
        return 1;
    }

    /* lseek(..., 0, SEEK_CUR) queries the current offset without changing it. */
    before = lseek(fd, 0, SEEK_CUR);
    if (before == (off_t) -1 || pread(fd, slice, 3, 4) != 3) {
        perror("pread");
        (void) close(fd);
        return 1;
    }

    /* pread() fetched bytes 4..6 but should have left the shared offset at zero. */
    after_pread = lseek(fd, 0, SEEK_CUR);
    if (after_pread == (off_t) -1 ||
        /* Replace bytes 1..3 without moving the shared offset. */
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

    /* The ordinary read starts at current offset zero and advances normally. */
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
