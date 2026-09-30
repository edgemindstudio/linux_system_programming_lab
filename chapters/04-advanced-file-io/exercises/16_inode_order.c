/*
 * Exercise 04.16 - Sort file requests by inode number
 *
 * Purpose:
 *   Demonstrate one historical user-space approximation for arranging related
 *   file requests before submitting them to the kernel.
 *
 * Linux behavior:
 *   fstat() exposes st_ino for a file descriptor. Sorting by inode does not
 *   reveal actual physical placement and is only a heuristic. On modern SSD,
 *   virtualized, networked, and copy-on-write storage it may have little value.
 */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct file_key {
    const char *path;
    ino_t inode;
};

static int compare_inode(const void *left, const void *right)
{
    const struct file_key *first = left;
    const struct file_key *second = right;

    if (first->inode < second->inode) {
        return -1;
    }
    if (first->inode > second->inode) {
        return 1;
    }
    return 0;
}

int main(void)
{
    struct file_key files[] = {
        { "build/chapters/04-advanced-file-io/data/inode_c.txt", 0 },
        { "build/chapters/04-advanced-file-io/data/inode_a.txt", 0 },
        { "build/chapters/04-advanced-file-io/data/inode_b.txt", 0 }
    };
    const size_t count = sizeof(files) / sizeof(files[0]);
    size_t index;

    for (index = 0; index < count; ++index) {
        struct stat info;
        int fd = open(files[index].path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd == -1) {
            perror("open inode demonstration file");
            return EXIT_FAILURE;
        }
        if (fstat(fd, &info) == -1) {
            perror("fstat inode demonstration file");
            (void) close(fd);
            return EXIT_FAILURE;
        }
        files[index].inode = info.st_ino;
        if (close(fd) == -1) {
            perror("close inode demonstration file");
            return EXIT_FAILURE;
        }
    }

    qsort(files, count, sizeof(files[0]), compare_inode);
    for (index = 1; index < count; ++index) {
        if (files[index - 1].inode > files[index].inode) {
            fputs("inode sort failed\n", stderr);
            return EXIT_FAILURE;
        }
    }

    printf("files=%zu ordered=yes strategy=inode-heuristic\n", count);
    for (index = 0; index < count; ++index) {
        printf("inode=%" PRIuMAX " path=%s\n",
               (uintmax_t) files[index].inode,
               files[index].path);
    }

    return EXIT_SUCCESS;
}
