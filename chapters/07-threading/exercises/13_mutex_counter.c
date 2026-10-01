/*
 * Exercise 07.13 - Protect a shared counter with a mutex
 *
 * Purpose:
 *   Make a read-modify-write critical region mutually exclusive.
 *
 * Linux behavior:
 *   counter++ is not an indivisible operation. Every worker therefore locks
 *   the mutex associated with the counter before incrementing it. The final
 *   count is deterministic after all workers are joined.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

enum { THREAD_COUNT = 4, INCREMENTS_PER_THREAD = 10000 };

struct protected_counter {
    pthread_mutex_t mutex;
    long value;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *increment_counter(void *argument)
{
    struct protected_counter *counter = argument;

    for (int index = 0; index < INCREMENTS_PER_THREAD; ++index) {
        (void)pthread_mutex_lock(&counter->mutex);
        ++counter->value;
        (void)pthread_mutex_unlock(&counter->mutex);
    }
    return NULL;
}

int main(void)
{
    struct protected_counter counter = {PTHREAD_MUTEX_INITIALIZER, 0};
    pthread_t threads[THREAD_COUNT];
    const long expected = (long)THREAD_COUNT * INCREMENTS_PER_THREAD;
    int created = 0;
    int result;

    for (int index = 0; index < THREAD_COUNT; ++index) {
        result = pthread_create(&threads[index], NULL, increment_counter, &counter);
        if (result != 0) {
            report_pthread_error("pthread_create", result);
            break;
        }
        ++created;
    }
    for (int index = 0; index < created; ++index) {
        result = pthread_join(threads[index], NULL);
        if (result != 0) {
            report_pthread_error("pthread_join", result);
            return EXIT_FAILURE;
        }
    }

    printf("expected=%ld actual=%ld\n", expected, counter.value);
    printf("critical_region_protected=%s all_threads_joined=%s\n",
           counter.value == expected ? "yes" : "no",
           created == THREAD_COUNT ? "yes" : "no");

    (void)pthread_mutex_destroy(&counter.mutex);
    return counter.value == expected && created == THREAD_COUNT
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
