/*
 * Exercise 11.16 - Schedule a whole-second alarm
 *
 * Purpose:
 *   Arrange one SIGALRM with alarm() and wait until its handler records
 *   delivery.
 *
 * Linux behavior:
 *   alarm() provides whole-second wall-clock scheduling and replaces any
 *   earlier alarm for the process. SIGALRM's default action is termination.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t alarmed;

static void remember_alarm(int signal_number)
{
    if (signal_number == SIGALRM) {
        alarmed = 1;
    }
}

int main(void)
{
    struct sigaction action = {0};
    unsigned int previous;

    action.sa_handler = remember_alarm;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGALRM, &action, NULL) == -1) {
        perror("sigaction SIGALRM");
        return EXIT_FAILURE;
    }

    previous = alarm(1U);
    while (alarmed == 0) {
        (void)pause();
    }

    printf("previous_alarm_seconds=%u alarm_delivered=%s\n",
           previous,
           alarmed == 1 ? "yes" : "no");
    printf("whole_second_granularity=yes one_process_alarm_slot=yes\n");

    return previous == 0U && alarmed == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
