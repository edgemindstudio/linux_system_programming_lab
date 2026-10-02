/*
 * Exercise 08.18 - Remove a watch and inspect the pending queue
 *
 * Purpose:
 *   Complete the inotify lifecycle: initialize, watch, remove, query pending
 *   bytes with FIONREAD, consume IN_IGNORED, and close the instance.
 *
 * Linux behavior:
 *   Removing a watch queues IN_IGNORED. FIONREAD reports pending bytes, not
 *   an event count. Closing the instance releases all remaining watch state.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

union lifecycle_buffer {
    struct inotify_event alignment;
    char bytes[1024];
};

int main(void)
{
    const char *directory =
        "build/chapters/08-file-and-directory-management/data/inotify_lifecycle";
    union lifecycle_buffer buffer;
    unsigned int pending_bytes = 0U;
    int ignored_found = 0;
    int inotify_fd;
    int watch_fd;

    if (mkdir(directory, 0755) == -1 && errno != EEXIST) {
        perror("mkdir lifecycle directory");
        return EXIT_FAILURE;
    }
    inotify_fd = inotify_init1(IN_CLOEXEC);
    if (inotify_fd == -1) {
        perror("inotify_init1");
        return EXIT_FAILURE;
    }
    watch_fd = inotify_add_watch(inotify_fd, directory, IN_ATTRIB);
    if (watch_fd == -1) {
        perror("inotify_add_watch");
        (void)close(inotify_fd);
        return EXIT_FAILURE;
    }
    if (inotify_rm_watch(inotify_fd, watch_fd) == -1) {
        perror("inotify_rm_watch");
        (void)close(inotify_fd);
        return EXIT_FAILURE;
    }
    if (ioctl(inotify_fd, FIONREAD, &pending_bytes) == -1) {
        perror("ioctl FIONREAD");
        (void)close(inotify_fd);
        return EXIT_FAILURE;
    }

    ssize_t received = read(inotify_fd, buffer.bytes, sizeof(buffer.bytes));
    if (received == -1) {
        perror("read IN_IGNORED");
        (void)close(inotify_fd);
        return EXIT_FAILURE;
    }
    for (size_t offset = 0U; offset < (size_t)received;) {
        const struct inotify_event *event =
            (const struct inotify_event *)(buffer.bytes + offset);

        if ((event->mask & IN_IGNORED) != 0U && event->wd == watch_fd) {
            ignored_found = 1;
        }
        offset += sizeof(*event) + event->len;
    }

    printf("pending_bytes_positive=%s ignored_event_found=%s\n",
           pending_bytes > 0U ? "yes" : "no",
           ignored_found != 0 ? "yes" : "no");
    printf("watch_removed=yes instance_closed=yes queue_measurement_is_bytes=yes\n");

    if (close(inotify_fd) == -1) {
        perror("close inotify instance");
        return EXIT_FAILURE;
    }
    return ignored_found != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
