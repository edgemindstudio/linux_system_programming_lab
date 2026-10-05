/*
 * Experiment - Real-time signals preserve queued occurrences
 *
 * Prediction:
 *   Three SIGRTMIN instances queued with different integer payloads remain
 *   individually available and arrive in send order.
 *
 * Observation boundary:
 *   Queue capacity is finite and subject to resource limits. This bounded
 *   experiment sends only three instances and checks every sigqueue() call.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum { ITEM_COUNT = 3 };

int main(void)
{
    const int expected[ITEM_COUNT] = {10, 20, 30};
    sigset_t wait_set;
    sigset_t previous_mask;
    int observed[ITEM_COUNT] = {0, 0, 0};
    int order_preserved = 1;
    int sum = 0;

    if (sigemptyset(&wait_set) == -1 ||
        sigaddset(&wait_set, SIGRTMIN) == -1 ||
        sigprocmask(SIG_BLOCK, &wait_set, &previous_mask) == -1) {
        perror("block SIGRTMIN");
        return EXIT_FAILURE;
    }

    for (int index = 0; index < ITEM_COUNT; ++index) {
        union sigval payload = {.sival_int = expected[index]};

        if (sigqueue(getpid(), SIGRTMIN, payload) == -1) {
            perror("sigqueue SIGRTMIN");
            return EXIT_FAILURE;
        }
    }

    for (int index = 0; index < ITEM_COUNT; ++index) {
        siginfo_t information;
        int received = sigwaitinfo(&wait_set, &information);

        if (received != SIGRTMIN) {
            perror("sigwaitinfo SIGRTMIN");
            return EXIT_FAILURE;
        }
        observed[index] = information.si_value.sival_int;
        sum += observed[index];
        if (observed[index] != expected[index]) {
            order_preserved = 0;
        }
    }

    if (sigprocmask(SIG_SETMASK, &previous_mask, NULL) == -1) {
        perror("restore signal mask");
        return EXIT_FAILURE;
    }

    printf("queued=3 received=3 values=%d,%d,%d sum=%d\n",
           observed[0], observed[1], observed[2], sum);
    printf("realtime_occurrences_preserved=yes order_preserved=%s\n",
           order_preserved ? "yes" : "no");

    return order_preserved && sum == 60 ? EXIT_SUCCESS : EXIT_FAILURE;
}
