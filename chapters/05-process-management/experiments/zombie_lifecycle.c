/*
 * Experiment — Observe a zombie and then reap it
 *
 * Prediction:
 *   A child that exits before its parent waits has state Z in /proc. After
 *   waitpid() consumes the status, that process directory disappears.
 *
 * Safety:
 *   The observation window is bounded and the program always reaps its child.
 *   A zombie holds termination metadata and a PID slot; it is not executing.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int process_state(pid_t pid, char *state)
{
    char path[64];
    char line[512];
    char *closing_parenthesis;
    FILE *stream;

    if (snprintf(path, sizeof(path), "/proc/%ld/stat", (long)pid) < 0) {
        return -1;
    }
    stream = fopen(path, "r");
    if (stream == NULL) {
        return -1;
    }
    if (fgets(line, sizeof(line), stream) == NULL) {
        (void)fclose(stream);
        return -1;
    }
    (void)fclose(stream);
    closing_parenthesis = strrchr(line, ')');
    if (closing_parenthesis == NULL || closing_parenthesis[1] != ' ' ||
        closing_parenthesis[2] == '\0') {
        return -1;
    }
    *state = closing_parenthesis[2];
    return 0;
}

int main(void)
{
    pid_t child = fork();
    int status;
    int zombie_observed = 0;
    char path[64];
    struct timespec pause_time = {.tv_sec = 0, .tv_nsec = 5000000L};

    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        _exit(17);
    }

    for (int attempt = 0; attempt < 200; ++attempt) {
        char state;

        if (process_state(child, &state) == 0 && state == 'Z') {
            zombie_observed = 1;
            break;
        }
        while (nanosleep(&pause_time, &pause_time) == -1 && errno == EINTR) {
        }
        pause_time.tv_sec = 0;
        pause_time.tv_nsec = 5000000L;
    }

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }
    if (snprintf(path, sizeof(path), "/proc/%ld", (long)child) < 0) {
        return EXIT_FAILURE;
    }

    printf("zombie_observed=%s exit_status=%d proc_entry_removed=%s\n",
           zombie_observed != 0 ? "yes" : "no",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1,
           access(path, F_OK) == -1 && errno == ENOENT ? "yes" : "no");
    return EXIT_SUCCESS;
}
