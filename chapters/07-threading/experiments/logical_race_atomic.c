/*
 * Experiment - An atomic variable can still participate in a logical race
 *
 * Prediction:
 *   Making the load and store individually atomic does not make the compound
 *   load-modify-store transaction atomic. A barrier deliberately aligns two
 *   workers so both read the same value before either stores.
 *
 * Safety:
 *   This program contains no C data race. C11 atomics keep every access
 *   defined; the experiment demonstrates a higher-level lost-update race.
 */

#define _XOPEN_SOURCE 700

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

enum { THREAD_COUNT = 2, ITERATIONS = 5000 };

struct race_state {
    atomic_int split_counter;
    atomic_int transaction_counter;
    pthread_barrier_t barrier;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *increment_counters(void *argument)
{
    struct race_state *state = argument;

    for (int index = 0; index < ITERATIONS; ++index) {
        int snapshot = atomic_load_explicit(&state->split_counter,
                                            memory_order_relaxed);

        (void)pthread_barrier_wait(&state->barrier);
        atomic_store_explicit(&state->split_counter,
                              snapshot + 1,
                              memory_order_relaxed);
        (void)pthread_barrier_wait(&state->barrier);
        (void)atomic_fetch_add_explicit(&state->transaction_counter,
                                        1,
                                        memory_order_relaxed);
    }
    return NULL;
}

int main(void)
{
    struct race_state state;
    pthread_t threads[THREAD_COUNT];
    const int expected = THREAD_COUNT * ITERATIONS;
    int result;

    atomic_init(&state.split_counter, 0);
    atomic_init(&state.transaction_counter, 0);
    result = pthread_barrier_init(&state.barrier, NULL, THREAD_COUNT);
    if (result != 0) {
        report_pthread_error("pthread_barrier_init", result);
        return EXIT_FAILURE;
    }

    for (int index = 0; index < THREAD_COUNT; ++index) {
        result = pthread_create(&threads[index], NULL, increment_counters, &state);
        if (result != 0) {
            report_pthread_error("pthread_create", result);
            return EXIT_FAILURE;
        }
    }
    for (int index = 0; index < THREAD_COUNT; ++index) {
        result = pthread_join(threads[index], NULL);
        if (result != 0) {
            report_pthread_error("pthread_join", result);
            return EXIT_FAILURE;
        }
    }

    printf("expected=%d split_load_store=%d atomic_fetch_add=%d\n",
           expected,
           atomic_load(&state.split_counter),
           atomic_load(&state.transaction_counter));
    printf("logical_race_observed=%s atomic_transaction_correct=%s\n",
           atomic_load(&state.split_counter) < expected ? "yes" : "no",
           atomic_load(&state.transaction_counter) == expected ? "yes" : "no");

    (void)pthread_barrier_destroy(&state.barrier);
    return EXIT_SUCCESS;
}
