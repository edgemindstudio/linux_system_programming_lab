/*
 * Experiment - Request Linux readahead explicitly
 *
 * Question:
 *   Does this filesystem accept the Linux-specific readahead() request?
 *
 * Interpretation:
 *   Success means the hint was accepted, not that storage completed before the
 *   call returned. Some filesystems may reject readahead; that is reported as
 *   an environmental observation rather than treated as a laboratory failure.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/04-advanced-file-io/data/readahead_probe.bin"

int main(void)
{
    const size_t length = 65536;
    int fd = open(OUTPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    ssize_t result;

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }
    if (ftruncate(fd, (off_t) length) == -1) {
        perror("ftruncate readahead file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    errno = 0;
    result = readahead(fd, 0, length);
    if (result == 0) {
        puts("readahead_result=accepted asynchronous_hint=yes");
    } else {
        printf("readahead_result=unavailable errno=%d description=%s\n",
               errno, strerror(errno));
    }

    if (close(fd) == -1) {
        perror("close " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
