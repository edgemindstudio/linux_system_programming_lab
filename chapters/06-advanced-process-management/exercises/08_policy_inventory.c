/*
 * Exercise 06.08 - Inspect scheduling policy and static priority
 *
 * Purpose:
 *   Read the policy and sched_param structure associated with this process.
 *
 * Linux behavior:
 *   Normal policies use a static priority of zero. SCHED_FIFO and SCHED_RR
 *   use positive real-time priorities. The nice value is separate from the
 *   sched_priority field and matters within normal scheduling classes.
 */

#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

static const char *name_policy(int policy)
{
    switch (policy) {
    case SCHED_OTHER:
        return "SCHED_OTHER";
    case SCHED_FIFO:
        return "SCHED_FIFO";
    case SCHED_RR:
        return "SCHED_RR";
#ifdef SCHED_BATCH
    case SCHED_BATCH:
        return "SCHED_BATCH";
#endif
#ifdef SCHED_IDLE
    case SCHED_IDLE:
        return "SCHED_IDLE";
#endif
    default:
        return "UNKNOWN";
    }
}

int main(void)
{
    struct sched_param parameters;
    int policy = sched_getscheduler(0);

    if (policy == -1) {
        perror("sched_getscheduler");
        return EXIT_FAILURE;
    }
    if (sched_getparam(0, &parameters) == -1) {
        perror("sched_getparam");
        return EXIT_FAILURE;
    }

    printf("policy=%s policy_number=%d static_priority=%d\n",
           name_policy(policy), policy, parameters.sched_priority);
    printf("policy_read=yes parameter_read=yes\n");

    return EXIT_SUCCESS;
}
