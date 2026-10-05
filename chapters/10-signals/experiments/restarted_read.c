/*
 * Experiment - SA_RESTART resumes a compatible blocking read
 *
 * Prediction:
 *   SIGUSR1 runs the handler, but read() remains logically in progress and
 *   returns the byte that the child writes afterward.
 *
 * Observation boundary:
 *   SA_RESTART does not restart every system call. Applications must still be
 *   designed to handle documented EINTR cases.
 */

#define _POSIX_C_SOURCE 200809L

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
    ssize_t read_result;
    int status;

    action.sa_handler = remember_signal;
    action.sa_flags = SA_RESTART;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGUSR1, &action, NULL) == -1 ||
        pipe(data_pipe) == -1) {
        perror("prepare restarted read");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        const char payload = 'S';

        (void)close(data_pipe[0]);
        (void)nanosleep(&delay, NULL);
        if (kill(getppid(), SIGUSR1) == -1) {
            _exit(2);
        }
        (void)nanosleep(&delay, NULL);
        _exit(write(data_pipe[1], &payload, 1U) == 1 ? 0 : 3);
    }

    (void)close(data_pipe[1]);
    read_result = read(data_pipe[0], &byte, 1U);
    (void)close(data_pipe[0]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    printf("handler_ran=%s read_returned_byte=%s byte=%c\n",
           handled == 1 ? "yes" : "no",
           read_result == 1 ? "yes" : "no",
           byte);
    printf("sa_restart_used=yes retry_visible_to_application=no "
           "child_exit_ok=%s\n",
           WIFEXITED(status) && WEXITSTATUS(status) == 0 ? "yes" : "no");

    return handled == 1 && read_result == 1 && byte == 'S' &&
                   WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
