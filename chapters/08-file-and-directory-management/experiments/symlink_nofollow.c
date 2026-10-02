/*
 * Experiment - Refuse a final symbolic-link traversal
 *
 * Prediction:
 *   A normal open() follows the link. O_NOFOLLOW rejects the same pathname
 *   when its final component is a symbolic link.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    const char *target =
        "build/chapters/08-file-and-directory-management/data/nofollow_target";
    const char *link_path =
        "build/chapters/08-file-and-directory-management/data/nofollow_link";
    int target_fd;
    int followed_fd;
    int protected_fd;
    int nofollow_rejected;

    (void)unlink(link_path);
    target_fd = open(target, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (target_fd == -1) {
        perror("open target");
        return EXIT_FAILURE;
    }
    if (close(target_fd) == -1 || symlink("nofollow_target", link_path) == -1) {
        perror("prepare symbolic link");
        return EXIT_FAILURE;
    }

    followed_fd = open(link_path, O_RDONLY | O_CLOEXEC);
    if (followed_fd == -1) {
        perror("open following symbolic link");
        return EXIT_FAILURE;
    }
    if (close(followed_fd) == -1) {
        perror("close followed target");
        return EXIT_FAILURE;
    }

    errno = 0;
    protected_fd = open(link_path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    nofollow_rejected = protected_fd == -1 && errno == ELOOP;
    if (protected_fd != -1) {
        (void)close(protected_fd);
    }

    printf("default_open_followed=yes nofollow_rejected_link=%s\n",
           nofollow_rejected != 0 ? "yes" : "no");
    printf("final_component_policy_explicit=yes race_resistance_requires_design=yes\n");

    return nofollow_rejected != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
