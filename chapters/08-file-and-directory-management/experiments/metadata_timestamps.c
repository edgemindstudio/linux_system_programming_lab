/*
 * Experiment - Separate access, modification, and status-change times
 *
 * Prediction:
 *   futimens() can assign access and modification times. A later permission
 *   change updates ctime because ctime records inode-status change, not birth.
 */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int main(void)
{
    const char *path =
        "build/chapters/08-file-and-directory-management/data/timestamps_demo";
    const struct timespec requested[2] = {
        {.tv_sec = 1000000000, .tv_nsec = 123456789},
        {.tv_sec = 1000000001, .tv_nsec = 987654321},
    };
    struct stat status;
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (futimens(fd, requested) == -1 || fchmod(fd, 0640) == -1 ||
        fstat(fd, &status) == -1) {
        perror("timestamp operations");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close");
        return EXIT_FAILURE;
    }

    printf("atime_set=%s mtime_set=%s\n",
           status.st_atim.tv_sec == requested[0].tv_sec ? "yes" : "no",
           status.st_mtim.tv_sec == requested[1].tv_sec ? "yes" : "no");
    printf("ctime_is_status_change=%s ctime_is_creation_time=no\n",
           status.st_ctim.tv_sec > status.st_mtim.tv_sec ? "yes" : "no");

    return EXIT_SUCCESS;
}
