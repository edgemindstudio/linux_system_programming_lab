/*
 * Exercise 08.10 - Read directory entries without assuming order
 *
 * Purpose:
 *   Traverse a directory stream with opendir(), readdir(), and closedir().
 *
 * Linux behavior:
 *   A directory maps names to inode references. readdir() order is not a
 *   sorting guarantee, and d_type may be unknown on some filesystems, so this
 *   exercise recognizes entries by name instead of position or d_type.
 */

#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int create_file(const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);

    if (fd == -1) {
        return -1;
    }
    return close(fd);
}

int main(void)
{
    const char *directory =
        "build/chapters/08-file-and-directory-management/data/readdir_demo";
    const char *alpha =
        "build/chapters/08-file-and-directory-management/data/readdir_demo/alpha";
    const char *beta =
        "build/chapters/08-file-and-directory-management/data/readdir_demo/beta";
    const char *gamma =
        "build/chapters/08-file-and-directory-management/data/readdir_demo/gamma";
    int found_alpha = 0;
    int found_beta = 0;
    int found_gamma = 0;
    DIR *stream;
    struct dirent *entry;

    if (mkdir(directory, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        return EXIT_FAILURE;
    }
    if (create_file(alpha) == -1 || create_file(beta) == -1) {
        perror("create directory entries");
        return EXIT_FAILURE;
    }
    if (mkdir(gamma, 0755) == -1 && errno != EEXIST) {
        perror("mkdir gamma");
        return EXIT_FAILURE;
    }

    stream = opendir(directory);
    if (stream == NULL) {
        perror("opendir");
        return EXIT_FAILURE;
    }
    errno = 0;
    while ((entry = readdir(stream)) != NULL) {
        if (strcmp(entry->d_name, "alpha") == 0) {
            found_alpha = 1;
        } else if (strcmp(entry->d_name, "beta") == 0) {
            found_beta = 1;
        } else if (strcmp(entry->d_name, "gamma") == 0) {
            found_gamma = 1;
        }
    }
    if (errno != 0) {
        perror("readdir");
        (void)closedir(stream);
        return EXIT_FAILURE;
    }
    if (closedir(stream) == -1) {
        perror("closedir");
        return EXIT_FAILURE;
    }

    printf("alpha=%s beta=%s gamma=%s\n",
           found_alpha != 0 ? "found" : "missing",
           found_beta != 0 ? "found" : "missing",
           found_gamma != 0 ? "found" : "missing");
    printf("all_expected_entries=%s order_assumed=no dtype_required=no\n",
           found_alpha != 0 && found_beta != 0 && found_gamma != 0
               ? "yes"
               : "no");

    return EXIT_SUCCESS;
}
