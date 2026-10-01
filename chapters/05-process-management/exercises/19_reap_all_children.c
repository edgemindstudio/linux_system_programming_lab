/*
 * Exercise 05.19 — Reap every child in a wait loop
 *
 * Purpose:
 *   Create several children and collect them with waitpid(-1, ...), without
 *   assuming which one exits first.
 *
 * Linux behavior:
 *   waitpid(-1, ...) selects any waitable child. Production supervisors repeat
 *   this pattern until no child statuses remain, often after SIGCHLD. The
 *   scheduler may choose any order, so this exercise verifies counts and the
 *   status sum instead of output order.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum { CHILD_COUNT = 3 };

int main(void)
{
    int created = 0;
    int reaped = 0;
    int status_sum = 0;

    for (int index = 0; index < CHILD_COUNT; ++index) {
        pid_t child = fork();

        if (child == -1) {
            perror("fork");
            return EXIT_FAILURE;
        }
        if (child == 0) {
            _exit(10 + index);
        }
        ++created;
    }

    while (reaped < created) {
        int status;
        pid_t child = waitpid(-1, &status, 0);

        if (child == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("waitpid");
            return EXIT_FAILURE;
        }
        if (!WIFEXITED(status)) {
            fprintf(stderr, "child did not exit normally\n");
            return EXIT_FAILURE;
        }
        status_sum += WEXITSTATUS(status);
        ++reaped;
    }

    printf("created=%d reaped=%d status_sum=%d all_collected=%s\n",
           created,
           reaped,
           status_sum,
           created == CHILD_COUNT && reaped == CHILD_COUNT && status_sum == 33
               ? "yes"
               : "no");
    return EXIT_SUCCESS;
}
