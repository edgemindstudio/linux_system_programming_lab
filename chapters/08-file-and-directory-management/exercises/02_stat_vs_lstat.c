/*
 * Exercise 08.02 - Compare stat() with lstat()
 *
 * Purpose:
 *   Observe the difference between asking about a symbolic link's target and
 *   asking about the symbolic link object itself.
 *
 * Linux behavior:
 *   stat() follows a symbolic link. lstat() stops at the link and reports its
 *   own inode, type, size, and timestamps.
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
    const char *target =
        "build/chapters/08-file-and-directory-management/data/stat_target.txt";
    const char *link_path =
        "build/chapters/08-file-and-directory-management/data/stat_link";
    struct stat target_status;
    struct stat followed_status;
    struct stat link_status;
    int fd;

    if (unlink(link_path) == -1 && errno != ENOENT) {
        perror("unlink old link");
        return EXIT_FAILURE;
    }
    fd = open(target, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd == -1) {
        perror("open target");
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close target");
        return EXIT_FAILURE;
    }
    if (symlink("stat_target.txt", link_path) == -1) {
        perror("symlink");
        return EXIT_FAILURE;
    }
    if (stat(target, &target_status) == -1 ||
        stat(link_path, &followed_status) == -1 ||
        lstat(link_path, &link_status) == -1) {
        perror("stat family");
        return EXIT_FAILURE;
    }

    printf("stat_follows_link=%s lstat_reports_link=%s\n",
           followed_status.st_ino == target_status.st_ino ? "yes" : "no",
           S_ISLNK(link_status.st_mode) ? "yes" : "no");
    printf("target_and_link_inodes_distinct=%s link_counted_as_object=yes\n",
           target_status.st_ino != link_status.st_ino ? "yes" : "no");

    return EXIT_SUCCESS;
}
