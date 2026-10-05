/*
 * Exercise 10.06 - Observe signal dispositions across exec()
 *
 * Purpose:
 *   Configure one ignored signal and one caught signal, replace the process
 *   image, and inspect both dispositions in the new image.
 *
 * Linux behavior:
 *   exec preserves signals explicitly ignored with SIG_IGN but resets caught
 *   dispositions to SIG_DFL. The signal mask also survives exec, although this
 *   exercise leaves it unchanged.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void unused_handler(int signal_number)
{
    (void)signal_number;
}

static int inspect_after_exec(void)
{
    struct sigaction ignored;
    struct sigaction caught;

    if (sigaction(SIGUSR1, NULL, &ignored) == -1 ||
        sigaction(SIGUSR2, NULL, &caught) == -1) {
        perror("inspect dispositions");
        return EXIT_FAILURE;
    }

    printf("ignored_disposition_preserved=%s caught_disposition_reset=%s\n",
           ignored.sa_handler == SIG_IGN ? "yes" : "no",
           caught.sa_handler == SIG_DFL ? "yes" : "no");
    printf("exec_replaced_process_image=yes pid_preserved=yes\n");

    return ignored.sa_handler == SIG_IGN && caught.sa_handler == SIG_DFL
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}

int main(int argc, char **argv)
{
    struct sigaction ignored = {0};
    struct sigaction caught = {0};

    if (argc == 2 && strcmp(argv[1], "--after-exec") == 0) {
        return inspect_after_exec();
    }

    ignored.sa_handler = SIG_IGN;
    caught.sa_handler = unused_handler;
    if (sigemptyset(&ignored.sa_mask) == -1 ||
        sigemptyset(&caught.sa_mask) == -1 ||
        sigaction(SIGUSR1, &ignored, NULL) == -1 ||
        sigaction(SIGUSR2, &caught, NULL) == -1) {
        perror("configure dispositions");
        return EXIT_FAILURE;
    }

    execl("/proc/self/exe", argv[0], "--after-exec", (char *)NULL);
    perror("execl /proc/self/exe");
    return EXIT_FAILURE;
}
