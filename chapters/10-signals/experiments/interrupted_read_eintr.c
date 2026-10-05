/*
 * Experiment - A caught signal interrupts a blocking read
 *
 * Prediction:
 *   With SA_RESTART disabled, SIGUSR1 interrupts read(), producing -1/EINTR.
 *   A retry then receives the byte written later by the child.
 *
 * Observation boundary:
 *   Restart behavior depends on the interface and handler flags. Correct code
 *   must understand the specific blocking operation and handle EINTR.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t handled;

static void remember_signal(int signal_number)
{
    if (signal_number == SIGUSR1) {
        handled = 1;
    }
}

int main(void)
{
    struct sigaction action = {0};
    const struct timespec delay = {.tv_sec = 0, .tv_nsec = 20000000L};
    int data_pipe[2];
    pid_t child;
    char byte = '\0';
    ssize_t first_result;
    int first_errno;
    ssize_t retry_result;
    int status;

    action.sa_handler = remember_signal;
    action.sa_flags = 0;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1 ||
        pipe(data_pipe) == -1) {
        perror("prepare interrupted read");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        const char payload = 'R';

        (void)close(data_pipe[0]);
        (void)nanosleep(&delay, NULL);
        if (kill(getppid(), SIGUSR1) == -1) {
            _exit(2);
        }
        (void)nanosleep(&delay, NULL);
        _exit(write(data_pipe[1], &payload, 1U) == 1 ? 0 : 3);
    }

    (void)close(data_pipe[1]);
    errno = 0;
    first_result = read(data_pipe[0], &byte, 1U);
    first_errno = errno;
    retry_result = read(data_pipe[0], &byte, 1U);
    (void)close(data_pipe[0]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("first_read_interrupted=%s errno_is_eintr=%s handler_ran=%s\n",
           first_result == -1 ? "yes" : "no",
           first_errno == EINTR ? "yes" : "no",
           handled == 1 ? "yes" : "no");
    printf("retry_received_byte=%s child_exit_ok=%s sa_restart=no\n",
           retry_result == 1 && byte == 'R' ? "yes" : "no",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no");

    return first_result == -1 && first_errno == EINTR && handled == 1 &&
                   retry_result == 1 && byte == 'R' && WIFEXITED(status) &&
                   WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
