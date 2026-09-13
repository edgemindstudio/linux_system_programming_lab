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
            continue;
        }

        if (written == -1 && errno == EINTR) {
            continue;
        }

        return -1;
    }

    return 0;
}

int main(void)
{
    const char message[] = "written through file descriptor 1 (stdout)\n";
    const char payload[] = "file descriptors are process-local handles\n";
    int first_fd;
    int second_fd;

    printf("standard descriptors: stdin=%d stdout=%d stderr=%d\n",
           STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
    fflush(stdout);

    if (write_all(STDOUT_FILENO, message, sizeof(message) - 1) == -1) {
        perror("write stdout");
        return 1;
    }

    first_fd = open("build/chapters/02-file-io/data/fd_lifecycle_a.txt",
                    O_WRONLY | O_CREAT | O_TRUNC,
                    0644);
    if (first_fd == -1) {
        perror("open fd_lifecycle_a.txt");
        return 1;
    }

    printf("first open returned fd=%d (often 3 in a simple process)\n", first_fd);

    if (write_all(first_fd, payload, sizeof(payload) - 1) == -1) {
        perror("write fd_lifecycle_a.txt");
        (void) close(first_fd);
        return 1;
    }

    if (close(first_fd) == -1) {
        perror("close fd_lifecycle_a.txt");
        return 1;
    }

    second_fd = open("build/chapters/02-file-io/data/fd_lifecycle_b.txt",
                     O_WRONLY | O_CREAT | O_TRUNC,
                     0644);
    if (second_fd == -1) {
        perror("open fd_lifecycle_b.txt");
        return 1;
    }

    printf("next open returned fd=%d; Linux may reuse a closed number\n",
           second_fd);

    if (close(second_fd) == -1) {
        perror("close fd_lifecycle_b.txt");
        return 1;
    }

    return 0;
}
