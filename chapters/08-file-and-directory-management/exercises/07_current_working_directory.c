/*
 * Exercise 08.07 - Change and restore the current working directory
 *
 * Purpose:
 *   Use getcwd(), chdir(), and fchdir() while keeping a descriptor-based route
 *   back to the original directory.
 *
 * Linux behavior:
 *   The current working directory belongs to process filesystem context. An
 *   open directory descriptor continues to identify that directory directly.
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
    const char *directory =
        "build/chapters/08-file-and-directory-management/data/cwd_demo";
    char before[PATH_MAX];
    char during[PATH_MAX];
    char after[PATH_MAX];
    int original_fd;

    if (rmdir(directory) == -1 && errno != ENOENT) {
        perror("rmdir old directory");
        return EXIT_FAILURE;
    }
    if (mkdir(directory, 0755) == -1) {
        perror("mkdir");
        return EXIT_FAILURE;
    }
    if (getcwd(before, sizeof(before)) == NULL) {
        perror("getcwd before");
        return EXIT_FAILURE;
    }
    original_fd = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (original_fd == -1) {
        perror("open current directory");
        return EXIT_FAILURE;
    }
    if (chdir(directory) == -1) {
        perror("chdir");
        (void)close(original_fd);
        return EXIT_FAILURE;
    }
    if (getcwd(during, sizeof(during)) == NULL) {
        perror("getcwd during");
        if (fchdir(original_fd) == -1) {
            perror("fchdir during error recovery");
        }
        if (close(original_fd) == -1) {
            perror("close directory descriptor during error recovery");
        }
        return EXIT_FAILURE;
    }
    if (fchdir(original_fd) == -1) {
        perror("fchdir");
        (void)close(original_fd);
        return EXIT_FAILURE;
    }
    if (close(original_fd) == -1) {
        perror("close directory descriptor");
        return EXIT_FAILURE;
    }
    if (getcwd(after, sizeof(after)) == NULL) {
        perror("getcwd after");
        return EXIT_FAILURE;
    }

    printf("directory_changed=%s directory_restored=%s\n",
           strcmp(before, during) != 0 ? "yes" : "no",
           strcmp(before, after) == 0 ? "yes" : "no");
    printf("descriptor_based_restore=yes cwd_is_process_context=yes\n");

    return EXIT_SUCCESS;
}
