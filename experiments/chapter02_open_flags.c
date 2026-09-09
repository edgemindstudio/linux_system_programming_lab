#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
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

static int write_then_close(int fd, const void *buffer, size_t count)
{
    int saved_errno = 0;
    int status = 0;

    if (write_all(fd, buffer, count) == -1) {
        saved_errno = errno;
        status = -1;
    }

    if (close(fd) == -1 && status == 0) {
        saved_errno = errno;
        status = -1;
    }

    if (status == -1) {
        errno = saved_errno;
    }

    return status;
}

static int display_file(const char *path)
{
    char buffer[128];
    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        return -1;
    }

    for (;;) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));

        if (bytes_read > 0) {
            if (write_all(STDOUT_FILENO, buffer, (size_t) bytes_read) == -1) {
                (void) close(fd);
                return -1;
            }
        } else if (bytes_read == 0) {
            break;
        } else if (errno != EINTR) {
            (void) close(fd);
            return -1;
        }
    }

    return close(fd);
}

int main(void)
{
    const char *path = "build/chapter02/data/open_flags_demo.txt";
    const char original[] = "original\n";
    const char appended[] = "appended despite seeking to offset zero\n";
    const char replacement[] = "replacement after O_TRUNC\n";
    int fd;

    if (unlink(path) == -1 && errno != ENOENT) {
        perror("unlink old demo file");
        return 1;
    }

    fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd == -1) {
        perror("exclusive create");
        return 1;
    }

    if (write_then_close(fd, original, sizeof(original) - 1) == -1) {
        perror("write/close original");
        return 1;
    }
    puts("O_CREAT | O_EXCL created the file once.");

    fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd == -1 && errno == EEXIST) {
        puts("A second exclusive create failed with EEXIST, as expected.");
    } else {
        if (fd != -1) {
            (void) close(fd);
        }
        fputs("unexpected result from second exclusive create\n", stderr);
        return 1;
    }

    fd = open(path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("open append");
        return 1;
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t) -1) {
        perror("lseek");
        (void) close(fd);
        return 1;
    }

    if (write_then_close(fd, appended, sizeof(appended) - 1) == -1) {
        perror("append/close");
        return 1;
    }

    puts("\nAfter O_APPEND write:");
    fflush(stdout);
    if (display_file(path) == -1) {
        perror("display after append");
        return 1;
    }

    fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror("open truncate");
        return 1;
    }

    if (write_then_close(fd, replacement, sizeof(replacement) - 1) == -1) {
        perror("replace/close");
        return 1;
    }

    puts("\nAfter O_TRUNC write:");
    fflush(stdout);
    if (display_file(path) == -1) {
        perror("display after truncate");
        return 1;
    }

    return 0;
}
