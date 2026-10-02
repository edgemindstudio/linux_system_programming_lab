/*
 * Exercise 08.01 - Read inode metadata with stat()
 *
 * Purpose:
 *   Create a regular file and inspect the metadata Linux associates with its
 *   inode: type, size, permissions, link count, and inode number.
 *
 * Linux behavior:
 *   A filename is a directory entry that refers to an inode. stat() follows
 *   that name to the inode and copies a metadata snapshot into struct stat.
 */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int write_all(int fd, const char *buffer, size_t length)
{
    size_t offset = 0U;

    while (offset < length) {
        ssize_t written = write(fd, buffer + offset, length - offset);

        if (written == -1) {
            return -1;
        }
        offset += (size_t)written;
    }
    return 0;
}

int main(void)
{
    const char *path =
        "build/chapters/08-file-and-directory-management/data/stat_demo.txt";
    const char content[] = "metadata\n";
    struct stat metadata;
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0640);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (write_all(fd, content, sizeof(content) - 1U) == -1) {
        perror("write");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("close");
        return EXIT_FAILURE;
    }
    if (stat(path, &metadata) == -1) {
        perror("stat");
        return EXIT_FAILURE;
    }

    printf("regular=%s size=%lld mode=%04o links=%lu\n",
           S_ISREG(metadata.st_mode) ? "yes" : "no",
           (long long)metadata.st_size,
           (unsigned int)(metadata.st_mode & 07777U),
           (unsigned long)metadata.st_nlink);
    printf("inode_positive=%s size_matches=%s metadata_is_snapshot=yes\n",
           metadata.st_ino > 0 ? "yes" : "no",
           metadata.st_size == (off_t)(sizeof(content) - 1U) ? "yes" : "no");

    return EXIT_SUCCESS;
}
