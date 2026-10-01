/*
 * Exercise 06.07 - Pin to one allowed CPU and restore the mask
 *
 * Purpose:
 *   Change this process's affinity without assuming that CPU 0 is available.
 *
 * Linux behavior:
 *   sched_setaffinity() intersects the requested mask with CPUs permitted by
 *   the current cpuset. We select the first CPU already allowed, verify the
 *   one-CPU mask, and restore the original mask before exiting.
 */

#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    cpu_set_t original;
    cpu_set_t one_cpu;
    cpu_set_t observed;
    cpu_set_t restored;
    size_t selected = (size_t)CPU_SETSIZE;
    size_t cpu;

    CPU_ZERO(&original);
    if (sched_getaffinity(0, sizeof(original), &original) == -1) {
        perror("sched_getaffinity original");
        return EXIT_FAILURE;
    }

    for (cpu = 0U; cpu < (size_t)CPU_SETSIZE; ++cpu) {
        if (CPU_ISSET(cpu, &original)) {
            selected = cpu;
            break;
        }
    }
    if (selected == (size_t)CPU_SETSIZE) {
        fputs("no allowed CPU found\n", stderr);
        return EXIT_FAILURE;
    }

    CPU_ZERO(&one_cpu);
    CPU_SET(selected, &one_cpu);
    if (sched_setaffinity(0, sizeof(one_cpu), &one_cpu) == -1) {
        perror("sched_setaffinity one CPU");
        return EXIT_FAILURE;
    }

    CPU_ZERO(&observed);
    if (sched_getaffinity(0, sizeof(observed), &observed) == -1) {
        perror("sched_getaffinity observed");
        return EXIT_FAILURE;
    }

    if (sched_setaffinity(0, sizeof(original), &original) == -1) {
        perror("sched_setaffinity restore");
        return EXIT_FAILURE;
    }

    CPU_ZERO(&restored);
    if (sched_getaffinity(0, sizeof(restored), &restored) == -1) {
        perror("sched_getaffinity restored");
        return EXIT_FAILURE;
    }

    printf("selected_cpu=%zu pinned_count=%d restored_count=%d\n",
           selected, CPU_COUNT(&observed), CPU_COUNT(&restored));
    printf("pin_verified=%s restore_verified=%s\n",
           CPU_COUNT(&observed) == 1 && CPU_ISSET(selected, &observed)
               ? "yes" : "no",
           CPU_EQUAL(&original, &restored) ? "yes" : "no");

    return EXIT_SUCCESS;
}
