#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapter02/data/seek_demo.txt"

static int write_all(int fd, const void *buffer, size_t count)
{
    const unsigned char *cursor = buffer;

    while (count > 0) {
        ssize_t bytes_written = write(fd, cursor, count);

        if (bytes_written > 0) {
            cursor += bytes_written;
            count -= (size_t) bytes_written;
        } else if (bytes_written == -1 && errno == EINTR) {
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
    const char replacement[] = "XY";
    char slice[4] = {0};
    char final[sizeof(initial)] = {0};
    off_t position;
    int fd = open(OUTPUT_PATH,
                  O_RDWR | O_CREAT | O_TRUNC,
                  0644);

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (write_all(fd, initial, sizeof(initial) - 1) == -1) {
        perror("write initial data");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    position = lseek(fd, -4, SEEK_CUR);
    if (position == (off_t) -1) {
        perror("lseek SEEK_CUR");
        (void) close(fd);
        return EXIT_FAILURE;
    }
    printf("After writing 10 bytes and seeking back 4: offset=%lld\n",
           (long long) position);

    if (read(fd, slice, 3) != 3) {
        perror("read slice");
        (void) close(fd);
        return EXIT_FAILURE;
    }
    printf("Read three bytes at offset 6: %s\n", slice);

    if (lseek(fd, 2, SEEK_SET) == (off_t) -1 ||
        write_all(fd, replacement, sizeof(replacement) - 1) == -1 ||
        lseek(fd, 0, SEEK_SET) == (off_t) -1) {
        perror("seek, replace, or rewind");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (read(fd, final, sizeof(initial) - 1) !=
        (ssize_t) (sizeof(initial) - 1)) {
        perror("read final file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    printf("Final file after writing XY at offset 2: %s\n", final);

    if (close(fd) == -1) {
        perror("close " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
