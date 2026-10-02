/*
 * Exercise 08.12 - Store and inspect a symbolic-link pathname
 *
 * Purpose:
 *   Create a symbolic link, read its stored pathname, and contrast its inode
 *   with the target inode.
 *
 * Linux behavior:
 *   A symbolic link is a separate filesystem object containing a pathname.
 *   readlink() does not append a terminating null byte for the caller.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    const char *target =
        "build/chapters/08-file-and-directory-management/data/symlink_target.txt";
    const char *link_path =
        "build/chapters/08-file-and-directory-management/data/symlink_demo";
    const char stored_target[] = "symlink_target.txt";
    char buffer[128];
    struct stat followed;
    struct stat link_status;
    int fd;

    (void)unlink(link_path);
    fd = open(target, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd == -1) {
        perror("open target");
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close target");
        return EXIT_FAILURE;
    }
    if (symlink(stored_target, link_path) == -1) {
        perror("symlink");
        return EXIT_FAILURE;
    }

    ssize_t length = readlink(link_path, buffer, sizeof(buffer) - 1U);
    if (length == -1) {
        perror("readlink");
        return EXIT_FAILURE;
    }
    buffer[(size_t)length] = '\0';
    if (stat(link_path, &followed) == -1 || lstat(link_path, &link_status) == -1) {
        perror("stat symbolic link");
        return EXIT_FAILURE;
    }

    printf("stored_path=%s path_matches=%s\n",
           buffer,
           strcmp(buffer, stored_target) == 0 ? "yes" : "no");
    printf("link_is_separate_inode=%s stat_followed_target=%s\n",
           S_ISLNK(link_status.st_mode) ? "yes" : "no",
           followed.st_ino != link_status.st_ino ? "yes" : "no");

    return EXIT_SUCCESS;
}
