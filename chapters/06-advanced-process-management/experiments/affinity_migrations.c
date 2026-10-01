/*
 * Experiment - Sample CPU placement and migrations
 *
 * Prediction:
 *   An unpinned task may be observed on one or several allowed CPUs. A quiet
 *   system may show no migration during this short observation window.
 */

#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    cpu_set_t seen;
    int previous = -1;
    unsigned long migrations = 0UL;
    unsigned long sample;

    CPU_ZERO(&seen);
    for (sample = 0UL; sample < 20000UL; ++sample) {
        int current = sched_getcpu();

        if (current == -1) {
            perror("sched_getcpu");
            return EXIT_FAILURE;
        }
        if ((size_t)current < (size_t)CPU_SETSIZE) {
            CPU_SET((size_t)current, &seen);
        }
        if (previous != -1 && current != previous) {
            ++migrations;
        }
        previous = current;
        if ((sample % 64UL) == 0UL && sched_yield() == -1) {
            perror("sched_yield");
            return EXIT_FAILURE;
        }
    }

    printf("samples=%lu distinct_cpus=%d migrations=%lu final_cpu=%d\n",
           sample, CPU_COUNT(&seen), migrations, previous);
    puts("sampling_completed=yes zero_migrations_is_valid=yes");
    return EXIT_SUCCESS;
}
