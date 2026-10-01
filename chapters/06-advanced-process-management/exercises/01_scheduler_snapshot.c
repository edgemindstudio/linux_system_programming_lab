/*
 * Exercise 06.01 - Snapshot the scheduler-visible process state
 *
 * Purpose:
 *   Connect one running process to three independent scheduling properties:
 *   its scheduling policy, its nice value, and the CPUs on which Linux may
 *   run it.
 *
 * Linux behavior:
 *   The policy selects a scheduling class. The nice value influences a normal
 *   process's relative CPU share. The affinity mask limits placement. None of
 *   these values is a promise that the process is running at this instant.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

static const char *policy_name(int policy)
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
    cpu_set_t allowed;
    int policy;
    int nice_value;
    int allowed_count;
    long online_count;

    policy = sched_getscheduler(0);
    if (policy == -1) {
        perror("sched_getscheduler");
        return EXIT_FAILURE;
    }

    errno = 0;
    nice_value = getpriority(PRIO_PROCESS, 0);
    if (nice_value == -1 && errno != 0) {
        perror("getpriority");
        return EXIT_FAILURE;
    }

    CPU_ZERO(&allowed);
    if (sched_getaffinity(0, sizeof(allowed), &allowed) == -1) {
        perror("sched_getaffinity");
        return EXIT_FAILURE;
    }

    allowed_count = CPU_COUNT(&allowed);
    online_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (online_count == -1) {
        perror("sysconf _SC_NPROCESSORS_ONLN");
        return EXIT_FAILURE;
    }

    printf("policy=%s nice=%d allowed_cpus=%d online_cpus=%ld\n",
           policy_name(policy), nice_value, allowed_count, online_count);
    printf("policy_known=%s allowed_cpu_positive=%s online_cpu_positive=%s\n",
           policy_name(policy)[0] != 'U' ? "yes" : "no",
           allowed_count > 0 ? "yes" : "no",
           online_count > 0 ? "yes" : "no");

    return EXIT_SUCCESS;
}
