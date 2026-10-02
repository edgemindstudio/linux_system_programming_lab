/*
 * Exercise 08.09 - Observe rmdir()'s empty-directory requirement
 *
 * Purpose:
 *   Attempt to remove a nonempty directory, then remove its entry and retry.
 *
 * Linux behavior:
 *   rmdir() removes directories, but refuses a directory containing entries
 *   other than dot and dot-dot. unlink() removes the regular file entry.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main(void)
{
    const char *directory =
        "build/chapters/08-file-and-directory-management/data/rmdir_demo";
    const char *file =
        "build/chapters/08-file-and-directory-management/data/rmdir_demo/entry";
    int fd;
    int nonempty_rejected;

    (void)unlink(file);
    (void)rmdir(directory);
    if (mkdir(directory, 0755) == -1) {
        perror("mkdir");
        return EXIT_FAILURE;
    }
    fd = open(file, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd == -1) {
        perror("open entry");
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close entry");
        return EXIT_FAILURE;
    }

    errno = 0;
    nonempty_rejected =
        rmdir(directory) == -1 && (errno == ENOTEMPTY || errno == EEXIST);

    if (unlink(file) == -1) {
        perror("unlink entry");
        return EXIT_FAILURE;
    }
    if (rmdir(directory) == -1) {
        perror("rmdir empty directory");
        return EXIT_FAILURE;
    }

    printf("nonempty_removal_rejected=%s empty_removal_succeeded=yes\n",
           nonempty_rejected != 0 ? "yes" : "no");
    printf("unlink_removes_entry_rmdir_removes_directory=yes\n");

    return EXIT_SUCCESS;
}
