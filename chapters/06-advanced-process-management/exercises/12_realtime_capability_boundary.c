/*
 * Exercise 06.12 - Inspect the real-time privilege boundary
 *
 * Purpose:
 *   Determine whether this process is configured to request a real-time
 *   priority without actually entering a real-time scheduling policy.
 *
 * Linux behavior:
 *   SCHED_FIFO and SCHED_RR can starve ordinary processes. Linux therefore
 *   normally requires CAP_SYS_NICE, while RLIMIT_RTPRIO may authorize a
 *   bounded priority for an unprivileged process. Observation is safe; this
 *   exercise intentionally does not call sched_setscheduler() for real time.
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

int main(void)
{
    struct rlimit limit;
    int fifo_min;
    int fifo_max;

    if (getrlimit(RLIMIT_RTPRIO, &limit) == -1) {
        perror("getrlimit RLIMIT_RTPRIO");
        return EXIT_FAILURE;
    }
    fifo_min = sched_get_priority_min(SCHED_FIFO);
    fifo_max = sched_get_priority_max(SCHED_FIFO);
    if (fifo_min == -1 || fifo_max == -1) {
        perror("sched_get_priority SCHED_FIFO");
        return EXIT_FAILURE;
    }

    printf("euid=%ld rlimit_rtprio_soft=%llu fifo_range=%d..%d\n",
           (long)geteuid(), (unsigned long long)limit.rlim_cur,
           fifo_min, fifo_max);
    printf("boundary_inspected=yes realtime_policy_changed=no "
           "limit_allows_realtime=%s\n",
           limit.rlim_cur != 0 ? "yes" : "no");

    return EXIT_SUCCESS;
}
