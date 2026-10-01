/*
 * Exercise 05.07 — exit() versus _exit()
 *
 * Purpose:
 *   Make the C-library cleanup performed by exit() visible. Two children put
 *   bytes in a fully buffered FILE stream; one calls exit(), while the other
 *   calls _exit() and bypasses stdio flushing.
 *
 * Linux behavior:
 *   exit() runs registered handlers and flushes open stdio output streams
 *   before requesting process termination. _exit() enters the kernel-facing
 *   termination path without that user-space cleanup. A post-fork child that
 *   cannot exec normally uses _exit() to avoid flushing inherited buffers.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define DATA_DIR "build/chapters/05-process-management/data"

static void child_write(const char *path, int use_exit)
{
    FILE *stream = fopen(path, "w");

    if (stream == NULL) {
        _exit(120);
    }
    if (setvbuf(stream, NULL, _IOFBF, BUFSIZ) != 0) {
        _exit(121);
    }
    if (fputs("buffered-data", stream) == EOF) {
        _exit(122);
    }

    if (use_exit != 0) {
        exit(23);
    }
    _exit(24);
}

static int run_child(const char *path, int use_exit, int expected_status)
{
    pid_t child = fork();
    int status;

    if (child == -1) {
        return -1;
    }
    if (child == 0) {
        child_write(path, use_exit);
    }
    if (waitpid(child, &status, 0) == -1) {
        return -1;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == expected_status ? 0 : -1;
}

int main(void)
{
    const char *exit_path = DATA_DIR "/exit_flushed.txt";
    const char *fast_path = DATA_DIR "/_exit_unflushed.txt";
    struct stat exit_stat;
    struct stat fast_stat;

    if (run_child(exit_path, 1, 23) == -1 ||
        run_child(fast_path, 0, 24) == -1) {
        perror("child lifecycle");
        return EXIT_FAILURE;
    }
    if (stat(exit_path, &exit_stat) == -1 || stat(fast_path, &fast_stat) == -1) {
        perror("stat");
        return EXIT_FAILURE;
    }

    printf("exit_bytes=%lld _exit_bytes=%lld\n",
           (long long)exit_stat.st_size,
           (long long)fast_stat.st_size);
    printf("exit_flushed=yes _exit_skipped_stdio=%s\n",
           fast_stat.st_size == 0 ? "yes" : "no");
    return EXIT_SUCCESS;
}
