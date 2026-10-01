/*
 * Exercise 06.15 - Observe RLIMIT_FSIZE enforcement in a child
 *
 * Purpose:
 *   Give a child a 1024-byte file-size ceiling and verify that Linux prevents
 *   the file from growing beyond it.
 *
 * Linux behavior:
 *   A write that would exceed RLIMIT_FSIZE is shortened or fails with EFBIG,
 *   and SIGXFSZ is generated. The child ignores SIGXFSZ so it can report the
 *   write failure through its exit status. The parent keeps its own limits.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int child_write_limited_file(const char *path)
{
    struct rlimit limit = {1024U, 1024U};
    unsigned char block[2048];
    ssize_t first;
    ssize_t second;
    int second_errno;
    int fd;

    memset(block, 'L', sizeof(block));
    if (setrlimit(RLIMIT_FSIZE, &limit) == -1) {
        return 10;
    }
    if (signal(SIGXFSZ, SIG_IGN) == SIG_ERR) {
        return 11;
    }

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        return 12;
    }

    first = write(fd, block, sizeof(block));
    errno = 0;
    second = write(fd, block, 1U);
    second_errno = errno;
    if (close(fd) == -1) {
        return 13;
    }

    if (first != 1024 || second != -1 || second_errno != EFBIG) {
        return 14;
    }
    return 0;
}

int main(void)
{
    const char *path =
        "build/chapters/06-advanced-process-management/data/limited_file.bin";
    struct stat information;
    int status;
    pid_t child = fork();

    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        _exit(child_write_limited_file(path));
    }

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }
    if (stat(path, &information) == -1) {
        perror("stat limited file");
        return EXIT_FAILURE;
    }

    printf("file_size=%lld child_exit=%d\n",
           (long long)information.st_size,
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    printf("size_limited=%s parent_survived=yes\n",
           information.st_size == 1024 && WIFEXITED(status) &&
                   WEXITSTATUS(status) == 0
               ? "yes" : "no");

    return EXIT_SUCCESS;
}
