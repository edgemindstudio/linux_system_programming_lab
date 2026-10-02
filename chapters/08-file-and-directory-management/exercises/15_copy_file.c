/*
 * Exercise 08.15 - Copy data into a distinct inode
 *
 * Purpose:
 *   Contrast copying with linking and renaming by transferring bytes into a
 *   newly created destination and deliberately preserving basic permissions.
 *
 * Linux behavior:
 *   Linux has no single universal "copy this pathname" operation. A program
 *   chooses how to transfer data and which metadata to reproduce.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
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
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        offset += (size_t)written;
    }
    return 0;
}

int main(void)
{
    const char *source_path =
        "build/chapters/08-file-and-directory-management/data/copy_source.txt";
    const char *destination_path =
        "build/chapters/08-file-and-directory-management/data/copy_destination.txt";
    const char source_content[] = "copy preserves selected metadata\n";
    char buffer[128];
    struct stat source_status;
    struct stat destination_status;
    size_t total = 0U;
    int source_fd;
    int destination_fd;

    source_fd = open(source_path,
                     O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC,
                     0600);
    if (source_fd == -1) {
        perror("open source");
        return EXIT_FAILURE;
    }
    if (write_all(source_fd, source_content, sizeof(source_content) - 1U) == -1 ||
        fchmod(source_fd, 0640) == -1 ||
        lseek(source_fd, 0, SEEK_SET) == (off_t)-1 ||
        fstat(source_fd, &source_status) == -1) {
        perror("prepare source");
        (void)close(source_fd);
        return EXIT_FAILURE;
    }

    destination_fd = open(destination_path,
                          O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC,
                          0600);
    if (destination_fd == -1) {
        perror("open destination");
        (void)close(source_fd);
        return EXIT_FAILURE;
    }

    for (;;) {
        ssize_t received = read(source_fd, buffer, sizeof(buffer));

        if (received == 0) {
            break;
        }
        if (received == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read source");
            return EXIT_FAILURE;
        }
        if (write_all(destination_fd, buffer, (size_t)received) == -1) {
            perror("write destination");
            return EXIT_FAILURE;
        }
        total += (size_t)received;
    }
    if (fchmod(destination_fd, source_status.st_mode & 0777U) == -1 ||
        fstat(destination_fd, &destination_status) == -1) {
        perror("preserve destination mode");
        return EXIT_FAILURE;
    }
    if (close(source_fd) == -1 || close(destination_fd) == -1) {
        perror("close copy descriptors");
        return EXIT_FAILURE;
    }

    printf("bytes_copied=%zu sizes_match=%s distinct_inodes=%s\n",
           total,
           source_status.st_size == destination_status.st_size ? "yes" : "no",
           source_status.st_ino != destination_status.st_ino ? "yes" : "no");
    printf("mode_preserved=%s metadata_policy_explicit=yes\n",
           (source_status.st_mode & 0777U) ==
                   (destination_status.st_mode & 0777U)
               ? "yes"
               : "no");

    return EXIT_SUCCESS;
}
