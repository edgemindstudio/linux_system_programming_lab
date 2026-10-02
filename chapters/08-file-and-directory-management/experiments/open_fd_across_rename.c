/*
 * Experiment - Follow an open descriptor across rename()
 *
 * Prediction:
 *   Renaming the pathname will not invalidate an already open descriptor.
 *
 * Observation rule:
 *   The descriptor and renamed pathname should report the same inode. Linux's
 *   /proc/self/fd view should resolve the descriptor using the current name.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    const char *before_path =
        "build/chapters/08-file-and-directory-management/data/fd_before.txt";
    const char *after_path =
        "build/chapters/08-file-and-directory-management/data/fd_after.txt";
    char proc_path[64];
    char resolved[PATH_MAX];
    struct stat descriptor_status;
    struct stat pathname_status;
    int fd;

    (void)unlink(before_path);
    (void)unlink(after_path);
    fd = open(before_path, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (rename(before_path, after_path) == -1) {
        perror("rename");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    if (fstat(fd, &descriptor_status) == -1 ||
        stat(after_path, &pathname_status) == -1) {
        perror("stat after rename");
        (void)close(fd);
        return EXIT_FAILURE;
    }

    int formatted = snprintf(proc_path, sizeof(proc_path), "/proc/self/fd/%d", fd);
    if (formatted < 0 || (size_t)formatted >= sizeof(proc_path)) {
        fputs("failed to format proc descriptor path\n", stderr);
        (void)close(fd);
        return EXIT_FAILURE;
    }
    ssize_t length = readlink(proc_path, resolved, sizeof(resolved) - 1U);
    if (length == -1) {
        perror("readlink /proc/self/fd");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    resolved[(size_t)length] = '\0';

    printf("descriptor_inode_matches_new_name=%s old_name_gone=%s\n",
           descriptor_status.st_ino == pathname_status.st_ino ? "yes" : "no",
           access(before_path, F_OK) == -1 && errno == ENOENT ? "yes" : "no");
    printf("proc_fd_mentions_new_name=%s inode_identity_survived=yes\n",
           strstr(resolved, "fd_after.txt") != NULL ? "yes" : "no");

    if (close(fd) == -1) {
        perror("close");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
