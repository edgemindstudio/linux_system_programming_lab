/*
 * Experiment - Observe an inode's hard-link lifecycle
 *
 * Prediction:
 *   Three names produce st_nlink == 3. Removing names decrements the count,
 *   while the last remaining name continues to expose the data.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    const char *first =
        "build/chapters/08-file-and-directory-management/data/link_life_a";
    const char *second =
        "build/chapters/08-file-and-directory-management/data/link_life_b";
    const char *third =
        "build/chapters/08-file-and-directory-management/data/link_life_c";
    const char content[] = "one inode\n";
    char buffer[32];
    struct stat status;
    nlink_t count_three;
    nlink_t count_two;
    nlink_t count_one;
    int fd;

    (void)unlink(first);
    (void)unlink(second);
    (void)unlink(third);
    fd = open(first, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (fd == -1) {
        perror("open first name");
        return EXIT_FAILURE;
    }
    if (write(fd, content, sizeof(content) - 1U) !=
        (ssize_t)(sizeof(content) - 1U)) {
        perror("write");
        return EXIT_FAILURE;
    }
    if (close(fd) == -1 || link(first, second) == -1 || link(first, third) == -1) {
        perror("create hard links");
        return EXIT_FAILURE;
    }
    if (stat(third, &status) == -1) {
        perror("stat three links");
        return EXIT_FAILURE;
    }
    count_three = status.st_nlink;
    if (unlink(second) == -1 || stat(third, &status) == -1) {
        perror("remove second link");
        return EXIT_FAILURE;
    }
    count_two = status.st_nlink;
    if (unlink(first) == -1 || stat(third, &status) == -1) {
        perror("remove first link");
        return EXIT_FAILURE;
    }
    count_one = status.st_nlink;

    fd = open(third, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        perror("open remaining link");
        return EXIT_FAILURE;
    }
    ssize_t received = read(fd, buffer, sizeof(buffer) - 1U);
    if (received == -1) {
        perror("read remaining link");
        return EXIT_FAILURE;
    }
    buffer[(size_t)received] = '\0';
    if (close(fd) == -1) {
        perror("close remaining link");
        return EXIT_FAILURE;
    }

    printf("link_counts=%lu,%lu,%lu\n",
           (unsigned long)count_three,
           (unsigned long)count_two,
           (unsigned long)count_one);
    printf("count_decremented=%s remaining_data_readable=%s\n",
           count_three == 3 && count_two == 2 && count_one == 1 ? "yes" : "no",
           strcmp(buffer, content) == 0 ? "yes" : "no");

    return EXIT_SUCCESS;
}
