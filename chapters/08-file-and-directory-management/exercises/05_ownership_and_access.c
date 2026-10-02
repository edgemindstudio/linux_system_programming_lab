/*
 * Exercise 08.05 - Inspect ownership and access identities
 *
 * Purpose:
 *   Connect inode owner/group fields to the process's real and effective
 *   credentials without requiring privileged ownership changes.
 *
 * Linux behavior:
 *   stat() reports the inode owner and group. access() answers using the real
 *   IDs, while most attempted file operations use effective credentials.
 */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    const char *path =
        "build/chapters/08-file-and-directory-management/data/ownership_demo.txt";
    struct stat status;
    uid_t real_uid = getuid();
    gid_t real_gid = getgid();
    uid_t effective_uid = geteuid();
    gid_t effective_gid = getegid();
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);

    if (fd == -1) {
        perror("open");
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

    printf("file_uid=%lu file_gid=%lu real_uid=%lu effective_uid=%lu\n",
           (unsigned long)status.st_uid,
           (unsigned long)status.st_gid,
           (unsigned long)real_uid,
           (unsigned long)effective_uid);
    printf("owned_by_real_user=%s real_group_matches=%s readable_writable=%s "
           "effective_group_recorded=%lu\n",
           status.st_uid == real_uid ? "yes" : "no",
           status.st_gid == real_gid ? "yes" : "no",
           access(path, R_OK | W_OK) == 0 ? "yes" : "no",
           (unsigned long)effective_gid);

    return EXIT_SUCCESS;
}
