/*
 * Exercise 08.08 - Apply umask while creating a directory
 *
 * Purpose:
 *   Predict the effective directory permissions from a requested mode and
 *   the process umask.
 *
 * Linux behavior:
 *   mkdir() requests permission bits, but the kernel clears every bit present
 *   in the process umask: effective_mode = requested_mode & ~umask.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    const char *path =
        "build/chapters/08-file-and-directory-management/data/mkdir_umask_demo";
    struct stat status;
    mode_t old_mask;

    if (rmdir(path) == -1 && errno != ENOENT) {
        perror("rmdir old directory");
        return EXIT_FAILURE;
    }

    old_mask = umask(0027);
    if (mkdir(path, 0777) == -1) {
        int saved_errno = errno;

        (void)umask(old_mask);
        errno = saved_errno;
        perror("mkdir");
        return EXIT_FAILURE;
    }
    (void)umask(old_mask);

    if (stat(path, &status) == -1) {
        perror("stat");
        return EXIT_FAILURE;
    }

    printf("requested=0777 umask=0027 observed=%04o\n",
           (unsigned int)(status.st_mode & 07777U));
    printf("effective_mode_correct=%s directory_created=yes\n",
           (status.st_mode & 0777U) == 0750U ? "yes" : "no");

    return EXIT_SUCCESS;
}
