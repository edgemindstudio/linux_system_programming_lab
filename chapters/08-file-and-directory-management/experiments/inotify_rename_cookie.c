/*
 * Experiment - Correlate both halves of an inotify rename
 *
 * Prediction:
 *   Renaming within one watched directory produces IN_MOVED_FROM and
 *   IN_MOVED_TO records carrying the same nonzero cookie.
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

union rename_event_buffer {
    struct inotify_event alignment;
    char bytes[4096];
};

int main(void)
{
    const char *directory =
        "build/chapters/08-file-and-directory-management/data/inotify_rename";
    const char *before =
        "build/chapters/08-file-and-directory-management/data/inotify_rename/before";
    const char *after =
        "build/chapters/08-file-and-directory-management/data/inotify_rename/after";
    union rename_event_buffer buffer;
    uint32_t from_cookie = 0U;
    uint32_t to_cookie = 0U;
    int from_found = 0;
    int to_found = 0;
    int inotify_fd;
    int watch_fd;
    int file_fd;

    (void)unlink(before);
    (void)unlink(after);
    if (mkdir(directory, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        return EXIT_FAILURE;
    }
    file_fd = open(before, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (file_fd == -1 || close(file_fd) == -1) {
        perror("create before entry");
        return EXIT_FAILURE;
    }
    inotify_fd = inotify_init1(IN_CLOEXEC);
    if (inotify_fd == -1) {
        perror("inotify_init1");
        return EXIT_FAILURE;
    }
    watch_fd = inotify_add_watch(inotify_fd, directory, IN_MOVED_FROM | IN_MOVED_TO);
    if (watch_fd == -1) {
        perror("inotify_add_watch");
        return EXIT_FAILURE;
    }
    if (rename(before, after) == -1) {
        perror("rename");
        return EXIT_FAILURE;
    }

    ssize_t received = read(inotify_fd, buffer.bytes, sizeof(buffer.bytes));
    if (received == -1) {
        perror("read rename events");
        return EXIT_FAILURE;
    }
    for (size_t offset = 0U; offset < (size_t)received;) {
        const struct inotify_event *event =
            (const struct inotify_event *)(buffer.bytes + offset);

        if ((event->mask & IN_MOVED_FROM) != 0U && event->len > 0U &&
            strcmp(event->name, "before") == 0) {
            from_found = 1;
            from_cookie = event->cookie;
        }
        if ((event->mask & IN_MOVED_TO) != 0U && event->len > 0U &&
            strcmp(event->name, "after") == 0) {
            to_found = 1;
            to_cookie = event->cookie;
        }
        offset += sizeof(*event) + event->len;
    }

    printf("moved_from_found=%s moved_to_found=%s\n",
           from_found != 0 ? "yes" : "no",
           to_found != 0 ? "yes" : "no");
    printf("cookies_match=%s cookie_nonzero=%s\n",
           from_cookie == to_cookie ? "yes" : "no",
           from_cookie != 0U ? "yes" : "no");

    if (close(inotify_fd) == -1) {
        perror("close inotify instance");
        return EXIT_FAILURE;
    }
    return from_found != 0 && to_found != 0 && from_cookie == to_cookie &&
                   from_cookie != 0U
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
