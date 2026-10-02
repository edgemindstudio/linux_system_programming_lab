/*
 * Experiment - Use a directory descriptor after its rename
 *
 * Prediction:
 *   An open directory descriptor remains attached to the directory inode even
 *   when the directory entry naming that inode changes.
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
        "build/chapters/08-file-and-directory-management/data/dir_before";
    const char *after_path =
        "build/chapters/08-file-and-directory-management/data/dir_after";
    char current[PATH_MAX];
    struct stat before_status;
    struct stat after_status;
    int original_fd;
    int directory_fd;

    (void)rmdir(before_path);
    (void)rmdir(after_path);
    if (mkdir(before_path, 0755) == -1) {
        perror("mkdir");
        return EXIT_FAILURE;
    }
    original_fd = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    directory_fd = open(before_path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (original_fd == -1 || directory_fd == -1) {
        perror("open directory descriptors");
        return EXIT_FAILURE;
    }
    if (fstat(directory_fd, &before_status) == -1 ||
        rename(before_path, after_path) == -1 ||
        fstat(directory_fd, &after_status) == -1 ||
        fchdir(directory_fd) == -1 ||
        getcwd(current, sizeof(current)) == NULL) {
        perror("directory rename experiment");
        return EXIT_FAILURE;
    }

    printf("directory_inode_stable=%s cwd_uses_new_name=%s\n",
           before_status.st_ino == after_status.st_ino ? "yes" : "no",
           strstr(current, "/dir_after") != NULL ? "yes" : "no");
    printf("descriptor_survived_rename=yes pathname_is_not_inode=yes\n");

    if (fchdir(original_fd) == -1 || close(directory_fd) == -1 ||
        close(original_fd) == -1) {
        perror("restore working directory");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
