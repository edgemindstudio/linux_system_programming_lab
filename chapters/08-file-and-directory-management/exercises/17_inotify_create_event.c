/*
 * Exercise 08.17 - Receive an inotify create event
 *
 * Purpose:
 *   Create an inotify instance, add one directory watch, trigger IN_CREATE,
 *   and parse the variable-length event records safely.
 *
 * Linux behavior:
 *   An inotify instance is a file descriptor. The kernel queues events for
 *   watched inodes, and read() returns one or more struct inotify_event records.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

union event_buffer {
    struct inotify_event alignment;
    char bytes[4096];
};

int main(void)
{
    const char *directory =
        "build/chapters/08-file-and-directory-management/data/inotify_create";
    const char *child =
        "build/chapters/08-file-and-directory-management/data/inotify_create/new_entry";
    union event_buffer buffer;
    int event_found = 0;
    int inotify_fd;
    int watch_fd;
    int child_fd;

    (void)unlink(child);
    if (mkdir(directory, 0755) == -1 && errno != EEXIST) {
        perror("mkdir watch directory");
        return EXIT_FAILURE;
    }
    inotify_fd = inotify_init1(IN_CLOEXEC);
    if (inotify_fd == -1) {
        perror("inotify_init1");
        return EXIT_FAILURE;
    }
    watch_fd = inotify_add_watch(inotify_fd, directory, IN_CREATE);
    if (watch_fd == -1) {
        perror("inotify_add_watch");
        (void)close(inotify_fd);
        return EXIT_FAILURE;
    }

    child_fd = open(child, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (child_fd == -1) {
        perror("create watched entry");
        (void)close(inotify_fd);
        return EXIT_FAILURE;
    }
    if (close(child_fd) == -1) {
        perror("close watched entry");
        (void)close(inotify_fd);
        return EXIT_FAILURE;
    }

    ssize_t received = read(inotify_fd, buffer.bytes, sizeof(buffer.bytes));
    if (received == -1) {
        perror("read inotify events");
        (void)close(inotify_fd);
        return EXIT_FAILURE;
    }
    for (size_t offset = 0U; offset < (size_t)received;) {
        const struct inotify_event *event =
            (const struct inotify_event *)(buffer.bytes + offset);

        if ((event->mask & IN_CREATE) != 0U && event->len > 0U &&
            strcmp(event->name, "new_entry") == 0) {
            event_found = 1;
        }
        offset += sizeof(*event) + event->len;
    }

    printf("watch_descriptor_nonnegative=%s create_event_found=%s\n",
           watch_fd >= 0 ? "yes" : "no",
           event_found != 0 ? "yes" : "no");
    printf("event_records_variable_length=yes inotify_is_fd_based=yes\n");

    if (close(inotify_fd) == -1) {
        perror("close inotify instance");
        return EXIT_FAILURE;
    }
    return event_found != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
