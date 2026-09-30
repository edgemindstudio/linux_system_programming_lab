/*
 * Exercise 04.14 - Give access-pattern advice for descriptor I/O
 *
 * Purpose:
 *   Mark a regular file for sequential near-future access.
 *
 * Linux behavior:
 *   posix_fadvise() returns an error number directly rather than returning -1
 *   and relying on errno. That unusual contract is why this program reports
 *   failures with strerror(result), not perror(). Advice remains only a hint.
 */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/04-advanced-file-io/data/fadvise_file.bin"

int main(void)
{
    int fd = open(OUTPUT_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
    int result;

    if (fd == -1) {
        perror("open " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (ftruncate(fd, 16384) == -1) {
        perror("ftruncate advised file");
        (void) close(fd);
        return EXIT_FAILURE;
    }

    result = posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    if (result != 0) {
        fprintf(stderr, "posix_fadvise sequential: %s\n", strerror(result));
        (void) close(fd);
        return EXIT_FAILURE;
    }

    result = posix_fadvise(fd, 0, 0, POSIX_FADV_WILLNEED);
    if (result != 0) {
        fprintf(stderr, "posix_fadvise willneed: %s\n", strerror(result));
        (void) close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1) {
        perror("close " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    puts("advice=sequential+willneed range=whole-file accepted=yes");
    return EXIT_SUCCESS;
}
