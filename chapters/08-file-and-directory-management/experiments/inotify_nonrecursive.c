/*
 * Experiment - Demonstrate that inotify directory watches are not recursive
 *
 * Prediction:
 *   Watching the parent reports creation of a child directory. Creating a
 *   file inside that child does not produce a named event on the parent watch.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

union recursive_event_buffer {
    struct inotify_event alignment;
    char bytes[4096];
};

int main(void)
{
    const char *parent =
        "build/chapters/08-file-and-directory-management/data/watch_parent";
    const char *child =
        "build/chapters/08-file-and-directory-management/data/watch_parent/child";
    const char *nested =
        "build/chapters/08-file-and-directory-management/data/watch_parent/child/nested";
    union recursive_event_buffer buffer;
    int child_seen = 0;
    int nested_seen = 0;
    int inotify_fd;
    int watch_fd;
    int nested_fd;

    (void)unlink(nested);
    (void)rmdir(child);
    if (mkdir(parent, 0755) == -1 && errno != EEXIST) {
        perror("mkdir parent");
        return EXIT_FAILURE;
    }
    inotify_fd = inotify_init1(IN_CLOEXEC);
    if (inotify_fd == -1) {
        perror("inotify_init1");
        return EXIT_FAILURE;
    }
    watch_fd = inotify_add_watch(inotify_fd, parent, IN_CREATE);
    if (watch_fd == -1) {
        perror("inotify_add_watch");
        return EXIT_FAILURE;
    }
    if (mkdir(child, 0755) == -1) {
        perror("mkdir child");
        return EXIT_FAILURE;
    }
    nested_fd = open(nested, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (nested_fd == -1 || close(nested_fd) == -1) {
        perror("create nested file");
        return EXIT_FAILURE;
    }

    ssize_t received = read(inotify_fd, buffer.bytes, sizeof(buffer.bytes));
    if (received == -1) {
        perror("read parent events");
        return EXIT_FAILURE;
    }
    for (size_t offset = 0U; offset < (size_t)received;) {
        const struct inotify_event *event =
            (const struct inotify_event *)(buffer.bytes + offset);

        if (event->len > 0U && strcmp(event->name, "child") == 0 &&
            (event->mask & IN_ISDIR) != 0U) {
            child_seen = 1;
        }
        if (event->len > 0U && strcmp(event->name, "nested") == 0) {
            nested_seen = 1;
        }
        offset += sizeof(*event) + event->len;
    }

    printf("child_directory_event=%s nested_file_event=%s\n",
           child_seen != 0 ? "yes" : "no",
           nested_seen != 0 ? "yes" : "no");
    printf("watch_is_nonrecursive=%s child_needs_own_watch=yes\n",
           child_seen != 0 && nested_seen == 0 ? "yes" : "no");

    if (close(inotify_fd) == -1) {
        perror("close inotify instance");
        return EXIT_FAILURE;
    }
    return child_seen != 0 && nested_seen == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
