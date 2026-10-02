/*
 * Exercise 08.06 - Complete an extended-attribute lifecycle
 *
 * Purpose:
 *   Set, read, list, and remove one user-namespace extended attribute.
 *
 * Linux behavior:
 *   Extended attributes attach name/value metadata to an inode. Filesystems
 *   or mounts may disable them, so an unsupported result is reported as an
 *   environmental capability rather than treated as a programming failure.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>

static int unsupported_xattr_error(int error_code)
{
    return error_code == ENOTSUP || error_code == EOPNOTSUPP ||
           error_code == EPERM;
}

int main(void)
{
    const char *path =
        "build/chapters/08-file-and-directory-management/data/xattr_demo.txt";
    const char *name = "user.chapter08";
    const char value[] = "metadata-lab";
    char received[64];
    char names[256];
    int found = 0;
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close");
        return EXIT_FAILURE;
    }
    if (setxattr(path, name, value, sizeof(value) - 1U, 0) == -1) {
        if (unsupported_xattr_error(errno)) {
            printf("xattr_supported=no reason=%s\n", strerror(errno));
            return EXIT_SUCCESS;
        }
        perror("setxattr");
        return EXIT_FAILURE;
    }

    ssize_t value_length = getxattr(path, name, received, sizeof(received) - 1U);
    if (value_length == -1) {
        perror("getxattr");
        return EXIT_FAILURE;
    }
    received[(size_t)value_length] = '\0';

    ssize_t names_length = listxattr(path, names, sizeof(names));
    if (names_length == -1) {
        perror("listxattr");
        return EXIT_FAILURE;
    }
    for (size_t offset = 0U; offset < (size_t)names_length;) {
        const char *current = names + offset;
        size_t current_length = strlen(current);

        if (strcmp(current, name) == 0) {
            found = 1;
        }
        offset += current_length + 1U;
    }

    if (removexattr(path, name) == -1) {
        perror("removexattr");
        return EXIT_FAILURE;
    }
    errno = 0;
    ssize_t after_removal = getxattr(path, name, received, sizeof(received));

    printf("xattr_supported=yes value=%s listed=%s\n",
           received,
           found != 0 ? "yes" : "no");
    printf("removed=%s inode_metadata_extended=yes\n",
           after_removal == -1 && errno == ENODATA ? "yes" : "no");

    return EXIT_SUCCESS;
}
