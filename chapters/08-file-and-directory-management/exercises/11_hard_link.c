/*
 * Exercise 08.11 - Create two names for one inode
 *
 * Purpose:
 *   Create a hard link and verify that both directory entries refer to the
 *   same inode while increasing its link count.
 *
 * Linux behavior:
 *   A hard link is another directory entry for an existing inode. Neither
 *   pathname is inherently the original after link() succeeds.
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
    const char *first =
        "build/chapters/08-file-and-directory-management/data/hard_first.txt";
    const char *second =
        "build/chapters/08-file-and-directory-management/data/hard_second.txt";
    struct stat first_status;
    struct stat second_status;
    int fd;

    if (unlink(second) == -1 && errno != ENOENT) {
        perror("unlink old hard link");
        return EXIT_FAILURE;
    }
    fd = open(first, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd == -1) {
        perror("open first");
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close first");
        return EXIT_FAILURE;
    }
    if (link(first, second) == -1) {
        perror("link");
        return EXIT_FAILURE;
    }
    if (stat(first, &first_status) == -1 || stat(second, &second_status) == -1) {
        perror("stat hard links");
        return EXIT_FAILURE;
    }

    printf("same_inode=%s first_links=%lu second_links=%lu\n",
           first_status.st_ino == second_status.st_ino ? "yes" : "no",
           (unsigned long)first_status.st_nlink,
           (unsigned long)second_status.st_nlink);
    printf("two_names_one_inode=%s hard_link_not_copy=yes\n",
           first_status.st_nlink >= 2 && second_status.st_nlink >= 2
               ? "yes"
               : "no");

    return EXIT_SUCCESS;
}
