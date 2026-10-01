/*
 * Experiment - Count observable mutex contention
 *
 * Prediction:
 *   A correct final counter does not imply that acquiring its lock was free.
 *   With several workers, some trylock attempts may find the mutex busy.
 *
 * Observation rule:
 *   The busy count depends on scheduling and may be zero. Correctness is the
 *   deterministic final counter; contention is an environment observation.
 */

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

enum { THREAD_COUNT = 4, ITERATIONS = 10000 };

struct contention_state {
    pthread_mutex_t mutex;
    long counter;
    atomic_ulong busy_observations;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *contend(void *argument)
{
    struct contention_state *state = argument;

    for (int index = 0; index < ITERATIONS; ++index) {
        int result = pthread_mutex_trylock(&state->mutex);

        if (result == EBUSY) {
            (void)atomic_fetch_add_explicit(&state->busy_observations,
                                            1UL,
                                            memory_order_relaxed);
            (void)pthread_mutex_lock(&state->mutex);
        } else if (result != 0) {
            return (void *)1;
        }
        ++state->counter;
        (void)pthread_mutex_unlock(&state->mutex);
    }
    return NULL;
}

int main(void)
{
    struct contention_state state = {
        PTHREAD_MUTEX_INITIALIZER,
        0,
        0UL,
    };
    pthread_t threads[THREAD_COUNT];
    const long expected = (long)THREAD_COUNT * ITERATIONS;
    int result;

    for (int index = 0; index < THREAD_COUNT; ++index) {
        result = pthread_create(&threads[index], NULL, contend, &state);
        if (result != 0) {
            report_pthread_error("pthread_create", result);
            return EXIT_FAILURE;
        }
    }
    for (int index = 0; index < THREAD_COUNT; ++index) {
        void *returned = NULL;

        result = pthread_join(threads[index], &returned);
        if (result != 0 || returned != NULL) {
            if (result != 0) {
                report_pthread_error("pthread_join", result);
            }
            return EXIT_FAILURE;
        }
    }

    printf("expected=%ld actual=%ld busy_observations=%lu\n",
           expected,
           state.counter,
           atomic_load(&state.busy_observations));
    printf("counter_correct=%s contention_is_environment_dependent=yes\n",
           state.counter == expected ? "yes" : "no");

    (void)pthread_mutex_destroy(&state.mutex);
    return state.counter == expected ? EXIT_SUCCESS : EXIT_FAILURE;
}
