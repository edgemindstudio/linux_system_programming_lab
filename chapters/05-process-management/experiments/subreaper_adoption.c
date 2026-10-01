/*
 * Experiment — Modern orphan adoption with a child subreaper
 *
 * Prediction:
 *   When the intermediate parent exits, its still-running child is reparented
 *   to the nearest living process marked as a child subreaper, not necessarily
 *   directly to PID 1.
 *
 * Linux behavior:
 *   PR_SET_CHILD_SUBREAPER is used by supervisors. The experiment blocks the
 *   grandchild on a pipe, reads its PPid from /proc, then releases and reaps it.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int exact_io(int fd, void *buffer, size_t length, int writing)
{
    unsigned char *cursor = buffer;

    while (length > 0U) {
        ssize_t count = writing != 0
                            ? write(fd, cursor, length)
                            : read(fd, cursor, length);
        if (count == 0 && writing == 0) {
            return -1;
        }
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        cursor += (size_t)count;
        length -= (size_t)count;
    }
    return 0;
}

static pid_t read_parent_pid(pid_t pid)
{
    char path[64];
    char line[256];
    FILE *stream;
    pid_t result = (pid_t)-1;

    if (snprintf(path, sizeof(path), "/proc/%ld/status", (long)pid) < 0) {
        return (pid_t)-1;
    }
    stream = fopen(path, "r");
    if (stream == NULL) {
        return (pid_t)-1;
    }
    while (fgets(line, sizeof(line), stream) != NULL) {
        long value;
        if (sscanf(line, "PPid:%ld", &value) == 1) {
            result = (pid_t)value;
            break;
        }
    }
    (void)fclose(stream);
    return result;
}

int main(void)
{
    int pid_pipe[2];
    int gate[2];
    pid_t intermediate;
    pid_t grandchild;
    pid_t observed_parent = (pid_t)-1;
    pid_t reaped_grandchild;
    pid_t supervisor = getpid();
    int status;
    char release = 'R';
    struct timespec pause_time = {.tv_sec = 0, .tv_nsec = 5000000L};

    if (prctl(PR_SET_CHILD_SUBREAPER, 1L) == -1) {
        if (errno == EINVAL) {
            puts("subreaper_supported=no");
            return EXIT_SUCCESS;
        }
        perror("prctl PR_SET_CHILD_SUBREAPER");
        return EXIT_FAILURE;
    }
    if (pipe(pid_pipe) == -1 || pipe(gate) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    intermediate = fork();
    if (intermediate == -1) {
        perror("fork intermediate");
        return EXIT_FAILURE;
    }
    if (intermediate == 0) {
        pid_t created_grandchild;

        (void)close(pid_pipe[0]);
        created_grandchild = fork();
        if (created_grandchild == -1) {
            _exit(120);
        }
        if (created_grandchild == 0) {
            char token;
            (void)close(pid_pipe[1]);
            (void)close(gate[1]);
            if (read(gate[0], &token, 1U) != 1) {
                _exit(121);
            }
            _exit(17);
        }

        (void)close(gate[0]);
        (void)close(gate[1]);
        if (exact_io(pid_pipe[1], &created_grandchild,
                     sizeof(created_grandchild), 1) == -1) {
            _exit(122);
        }
        (void)close(pid_pipe[1]);
        _exit(0);
    }

    (void)close(pid_pipe[1]);
    (void)close(gate[0]);
    if (exact_io(pid_pipe[0], &grandchild, sizeof(grandchild), 0) == -1) {
        perror("read grandchild pid");
        return EXIT_FAILURE;
    }
    (void)close(pid_pipe[0]);
    if (waitpid(intermediate, &status, 0) == -1 ||
        !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "intermediate child failed\n");
        return EXIT_FAILURE;
    }

    for (int attempt = 0; attempt < 200; ++attempt) {
        observed_parent = read_parent_pid(grandchild);
        if (observed_parent == supervisor) {
            break;
        }
        while (nanosleep(&pause_time, &pause_time) == -1 && errno == EINTR) {
        }
        pause_time.tv_sec = 0;
        pause_time.tv_nsec = 5000000L;
    }

    if (write(gate[1], &release, 1U) != 1) {
        perror("release grandchild");
        return EXIT_FAILURE;
    }
    (void)close(gate[1]);
    reaped_grandchild = waitpid(grandchild, &status, 0);
    if (reaped_grandchild == -1) {
        perror("waitpid grandchild");
        return EXIT_FAILURE;
    }

    printf("subreaper_supported=yes adopted_by_subreaper=%s proc_parent_matches=%s ",
           reaped_grandchild == grandchild ? "yes" : "no",
           observed_parent == supervisor ? "yes" : "no");
    printf("grandchild_exit=%d\n",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    return EXIT_SUCCESS;
}
