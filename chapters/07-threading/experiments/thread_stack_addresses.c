/*
 * Experiment - Compare stack-local and shared addresses
 *
 * Prediction:
 *   Each thread's automatic object lives at a distinct stack address, while
 *   every thread receives the same address for one shared heap object.
 */

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { THREAD_COUNT = 4 };

struct address_report {
    uintptr_t local_address;
    uintptr_t shared_address;
};

struct worker_argument {
    struct address_report *report;
    int *shared_value;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *capture_addresses(void *argument)
{
    struct worker_argument *worker = argument;
    int local_value = 0;

    worker->report->local_address = (uintptr_t)&local_value;
    worker->report->shared_address = (uintptr_t)worker->shared_value;
    return NULL;
}

int main(void)
{
    pthread_t threads[THREAD_COUNT];
    struct address_report reports[THREAD_COUNT] = {{0U, 0U}};
    struct worker_argument arguments[THREAD_COUNT];
    int *shared_value = malloc(sizeof(*shared_value));
    int result;
    int local_addresses_distinct = 1;
    int shared_address_identical = 1;

    if (shared_value == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    *shared_value = 42;

    for (int index = 0; index < THREAD_COUNT; ++index) {
        arguments[index].report = &reports[index];
        arguments[index].shared_value = shared_value;
        result = pthread_create(&threads[index], NULL, capture_addresses,
                                &arguments[index]);
        if (result != 0) {
            report_pthread_error("pthread_create", result);
            free(shared_value);
            return EXIT_FAILURE;
        }
    }
    for (int index = 0; index < THREAD_COUNT; ++index) {
        result = pthread_join(threads[index], NULL);
        if (result != 0) {
            report_pthread_error("pthread_join", result);
            free(shared_value);
            return EXIT_FAILURE;
        }
    }

    for (int left = 0; left < THREAD_COUNT; ++left) {
        if (reports[left].shared_address != (uintptr_t)shared_value) {
            shared_address_identical = 0;
        }
        for (int right = left + 1; right < THREAD_COUNT; ++right) {
            if (reports[left].local_address == reports[right].local_address) {
                local_addresses_distinct = 0;
            }
        }
    }

    printf("threads=%d local_addresses_distinct=%s\n",
           THREAD_COUNT,
           local_addresses_distinct != 0 ? "yes" : "no");
    printf("shared_heap_address_identical=%s shared_value=%d\n",
           shared_address_identical != 0 ? "yes" : "no",
           *shared_value);

    free(shared_value);
    return local_addresses_distinct != 0 && shared_address_identical != 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
