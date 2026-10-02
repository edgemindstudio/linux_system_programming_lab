/*
 * Exercise 08.04 - Change permissions through an open descriptor
 *
 * Purpose:
 *   Use fchmod() when a program already owns a descriptor for the object it
 *   intends to modify.
 *
 * Linux behavior:
 *   fchmod() changes inode metadata. Unlike creation mode processing, its
 *   requested permission bits are not filtered through the process umask.
 */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    const char *path =
        "build/chapters/08-file-and-directory-management/data/fchmod_demo.txt";
    struct stat status;
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (fchmod(fd, 0640) == -1) {
        perror("fchmod");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    if (fstat(fd, &status) == -1) {
        perror("fstat");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close");
        return EXIT_FAILURE;
    }

    printf("requested=0640 observed=%04o\n",
           (unsigned int)(status.st_mode & 07777U));
    printf("permissions_changed=%s descriptor_target_stable=yes\n",
           (status.st_mode & 0777U) == 0640U ? "yes" : "no");

    return EXIT_SUCCESS;
}
