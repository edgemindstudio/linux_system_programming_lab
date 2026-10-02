/*
 * Exercise 08.14 - Rename a directory entry without copying data
 *
 * Purpose:
 *   Verify that rename() changes the pathname while preserving the inode.
 *
 * Linux behavior:
 *   Within one filesystem, rename() updates namespace entries atomically. A
 *   successful call does not copy the file contents into a new inode.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    const char *old_path =
        "build/chapters/08-file-and-directory-management/data/rename_before.txt";
    const char *new_path =
        "build/chapters/08-file-and-directory-management/data/rename_after.txt";
    struct stat before;
    struct stat after;
    int old_name_gone;
    int fd;

    (void)unlink(old_path);
    (void)unlink(new_path);
    fd = open(old_path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (fd == -1) {
        perror("open old path");
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close old path");
        return EXIT_FAILURE;
    }
    if (stat(old_path, &before) == -1) {
        perror("stat before");
        return EXIT_FAILURE;
    }
    if (rename(old_path, new_path) == -1) {
        perror("rename");
        return EXIT_FAILURE;
    }
    errno = 0;
    old_name_gone = lstat(old_path, &after) == -1 && errno == ENOENT;
    if (stat(new_path, &after) == -1) {
        perror("stat after");
        return EXIT_FAILURE;
    }

    printf("old_name_gone=%s new_name_exists=yes same_inode=%s\n",
           old_name_gone != 0 ? "yes" : "no",
           before.st_ino == after.st_ino ? "yes" : "no");
    printf("namespace_changed_without_copy=%s same_filesystem_required=yes\n",
           before.st_dev == after.st_dev && before.st_ino == after.st_ino
               ? "yes"
               : "no");

    return EXIT_SUCCESS;
}
