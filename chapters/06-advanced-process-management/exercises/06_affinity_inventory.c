/*
 * Exercise 06.06 - Inspect the allowed CPU affinity mask
 *
 * Purpose:
 *   Enumerate the logical CPUs on which the current process may run.
 *
 * Linux behavior:
 *   The affinity mask can be narrower than the machine's online CPU set due
 *   to taskset, containers, cpusets, systemd units, or administrator policy.
 *   CPU numbers are identifiers, not a promise of continuous execution.
 */

#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    cpu_set_t allowed;
    int count;
    size_t first = CPU_SETSIZE;
    size_t last = CPU_SETSIZE;
    size_t cpu;

    CPU_ZERO(&allowed);
    if (sched_getaffinity(0, sizeof(allowed), &allowed) == -1) {
        perror("sched_getaffinity");
        return EXIT_FAILURE;
    }

    count = CPU_COUNT(&allowed);
    for (cpu = 0U; cpu < (size_t)CPU_SETSIZE; ++cpu) {
        if (CPU_ISSET(cpu, &allowed)) {
            if (first == (size_t)CPU_SETSIZE) {
                first = cpu;
            }
            last = cpu;
        }
    }

    printf("allowed_count=%d first_cpu=%zu last_cpu=%zu\n", count, first, last);
    printf("mask_nonempty=%s identifiers_ordered=%s\n",
           count > 0 ? "yes" : "no",
           first != (size_t)CPU_SETSIZE && last >= first ? "yes" : "no");

    return EXIT_SUCCESS;
}
