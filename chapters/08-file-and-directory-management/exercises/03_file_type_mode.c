/*
 * Exercise 08.03 - Decode file type bits in st_mode
 *
 * Purpose:
 *   Create three filesystem object types and identify them with the S_IS...
 *   macros rather than by comparing implementation-specific bit patterns.
 *
 * Linux behavior:
 *   st_mode contains both the file type and permission bits. A directory and
 *   a FIFO have inodes and metadata even though neither is a regular file.
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
    const char *regular_path =
        "build/chapters/08-file-and-directory-management/data/type_regular";
    const char *directory_path =
        "build/chapters/08-file-and-directory-management/data/type_directory";
    const char *fifo_path =
        "build/chapters/08-file-and-directory-management/data/type_fifo";
    struct stat regular_status;
    struct stat directory_status;
    struct stat fifo_status;
    int fd;

    (void)unlink(fifo_path);
    (void)rmdir(directory_path);

    fd = open(regular_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd == -1) {
        perror("open regular file");
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close regular file");
        return EXIT_FAILURE;
    }
    if (mkdir(directory_path, 0755) == -1) {
        perror("mkdir");
        return EXIT_FAILURE;
    }
    if (mkfifo(fifo_path, 0600) == -1) {
        perror("mkfifo");
        return EXIT_FAILURE;
    }
    if (lstat(regular_path, &regular_status) == -1 ||
        lstat(directory_path, &directory_status) == -1 ||
        lstat(fifo_path, &fifo_status) == -1) {
        perror("lstat");
        return EXIT_FAILURE;
    }

    printf("regular_detected=%s directory_detected=%s fifo_detected=%s\n",
           S_ISREG(regular_status.st_mode) ? "yes" : "no",
           S_ISDIR(directory_status.st_mode) ? "yes" : "no",
           S_ISFIFO(fifo_status.st_mode) ? "yes" : "no");
    printf("portable_type_macros_used=yes mode_contains_type_and_permissions=yes\n");

    return EXIT_SUCCESS;
}
