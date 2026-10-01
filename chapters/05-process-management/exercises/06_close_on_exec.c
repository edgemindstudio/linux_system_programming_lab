/*
 * Exercise 05.06 — Descriptor inheritance and FD_CLOEXEC
 *
 * Purpose:
 *   Compare one ordinary descriptor with another marked close-on-exec, then
 *   replace the process image and inspect both descriptors in the new program.
 *
 * Linux behavior:
 *   Open descriptors survive exec by default. FD_CLOEXEC tells the kernel to
 *   close a descriptor atomically during a successful exec, preventing an
 *   unintended capability or resource from leaking into the new program.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int parse_fd(const char *text)
{
    char *end = NULL;
    long value = strtol(text, &end, 10);

    if (end == text || *end != '\0' || value < 0 || value > 1024) {
        return -1;
    }
    return (int)value;
}

static int inspect_after_exec(const char *kept_text, const char *closed_text)
{
    int kept_fd = parse_fd(kept_text);
    int closed_fd = parse_fd(closed_text);
    int kept_result;
    int closed_result;
    int closed_errno;

    if (kept_fd == -1 || closed_fd == -1) {
        fprintf(stderr, "invalid descriptor argument\n");
        return EXIT_FAILURE;
    }

    errno = 0;
    kept_result = fcntl(kept_fd, F_GETFD);
    errno = 0;
    closed_result = fcntl(closed_fd, F_GETFD);
    closed_errno = errno;

    printf("ordinary_inherited=%s cloexec_closed=%s\n",
           kept_result != -1 ? "yes" : "no",
           closed_result == -1 && closed_errno == EBADF ? "yes" : "no");

    if (kept_result != -1) {
        (void)close(kept_fd);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    int kept_fd;
    int cloexec_fd;
    int flags;
    char kept_text[32];
    char closed_text[32];
    char *child_argv[] = {
        "06_close_on_exec",
        "--inspect",
        kept_text,
        closed_text,
        NULL,
    };

    if (argc == 4 && strcmp(argv[1], "--inspect") == 0) {
        return inspect_after_exec(argv[2], argv[3]);
    }

    kept_fd = open("/dev/null", O_RDONLY);
    cloexec_fd = open("/dev/null", O_RDONLY);
    if (kept_fd == -1 || cloexec_fd == -1) {
        perror("open /dev/null");
        return EXIT_FAILURE;
    }

    flags = fcntl(cloexec_fd, F_GETFD);
    if (flags == -1 || fcntl(cloexec_fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        perror("fcntl FD_CLOEXEC");
        return EXIT_FAILURE;
    }

    if (snprintf(kept_text, sizeof(kept_text), "%d", kept_fd) < 0 ||
        snprintf(closed_text, sizeof(closed_text), "%d", cloexec_fd) < 0) {
        return EXIT_FAILURE;
    }

    execv("/proc/self/exe", child_argv);
    perror("execv");
    return EXIT_FAILURE;
}
