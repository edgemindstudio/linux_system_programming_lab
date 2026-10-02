/*
 * Exercise 08.13 - Keep using an inode after unlinking its name
 *
 * Purpose:
 *   Show that unlink() removes a directory entry, not an already open file
 *   description or the inode while references remain.
 *
 * Linux behavior:
 *   After the final pathname is unlinked, st_nlink becomes zero. The open
 *   descriptor keeps the inode and its data alive until close().
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
    const char *path =
        "build/chapters/08-file-and-directory-management/data/unlink_open.txt";
    const char content[] = "still-readable\n";
    char buffer[64];
    struct stat status;
    int name_gone;
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (write(fd, content, sizeof(content) - 1U) !=
        (ssize_t)(sizeof(content) - 1U)) {
        perror("write");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    if (unlink(path) == -1) {
        perror("unlink");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    errno = 0;
    name_gone = access(path, F_OK) == -1 && errno == ENOENT;

    if (fstat(fd, &status) == -1) {
        perror("fstat");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    ssize_t received = read(fd, buffer, sizeof(buffer) - 1U);
    if (received == -1) {
        perror("read");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    buffer[(size_t)received] = '\0';

    printf("name_gone=%s link_count=%lu data=%s",
           name_gone != 0 ? "yes" : "no",
           (unsigned long)status.st_nlink,
           buffer);
    printf("descriptor_kept_inode_alive=%s\n",
           status.st_nlink == 0 && strcmp(buffer, content) == 0 ? "yes" : "no");

    if (close(fd) == -1) {
        perror("close final reference");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
