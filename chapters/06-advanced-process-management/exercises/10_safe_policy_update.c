/*
 * Exercise 06.10 - Make a safe no-op scheduler update
 *
 * Purpose:
 *   Practice sched_setscheduler() without entering a real-time class.
 *
 * Linux behavior:
 *   Reapplying the current normal scheduling policy with its existing static
 *   priority exercises the API but does not elevate the process. Entering
 *   SCHED_FIFO or SCHED_RR is deliberately excluded because a runaway
 *   real-time task can starve ordinary work.
 */

#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

static int is_safe_normal_policy(int policy)
{
    if (policy == SCHED_OTHER) {
        return 1;
    }
#ifdef SCHED_BATCH
    if (policy == SCHED_BATCH) {
        return 1;
    }
#endif
#ifdef SCHED_IDLE
    if (policy == SCHED_IDLE) {
        return 1;
    }
#endif
    return 0;
}

int main(void)
{
    struct sched_param before_parameters;
    struct sched_param after_parameters;
    int before_policy;
    int after_policy;

    before_policy = sched_getscheduler(0);
    if (before_policy == -1) {
        perror("sched_getscheduler before");
        return EXIT_FAILURE;
    }
    if (!is_safe_normal_policy(before_policy)) {
        printf("safe_update_skipped=yes reason=already-realtime policy=%d\n",
               before_policy);
        return EXIT_SUCCESS;
    }
    if (sched_getparam(0, &before_parameters) == -1) {
        perror("sched_getparam before");
        return EXIT_FAILURE;
    }
    if (sched_setscheduler(0, before_policy, &before_parameters) == -1) {
        perror("sched_setscheduler same policy");
        return EXIT_FAILURE;
    }

    after_policy = sched_getscheduler(0);
    if (after_policy == -1 || sched_getparam(0, &after_parameters) == -1) {
        perror("scheduler query after");
        return EXIT_FAILURE;
    }

    printf("before_policy=%d after_policy=%d static_priority=%d\n",
           before_policy, after_policy, after_parameters.sched_priority);
    printf("safe_update_applied=yes policy_unchanged=%s priority_unchanged=%s\n",
           before_policy == after_policy ? "yes" : "no",
           before_parameters.sched_priority == after_parameters.sched_priority
               ? "yes" : "no");

    return EXIT_SUCCESS;
}
