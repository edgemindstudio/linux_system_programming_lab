/*
 * Exercise 07.18 - Wait for a state predicate with a condition variable
 *
 * Purpose:
 *   Coordinate a producer and worker without polling or assuming a scheduling
 *   order.
 *
 * Linux behavior:
 *   pthread_cond_wait() atomically releases the mutex and sleeps. After it is
 *   awakened, it reacquires the mutex and the program rechecks the predicate
 *   in a loop because wakeups do not themselves prove the state is ready.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct handoff {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int ready;
    int value;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *consume_value(void *argument)
{
    struct handoff *handoff = argument;

    (void)pthread_mutex_lock(&handoff->mutex);
    while (handoff->ready == 0) {
        (void)pthread_cond_wait(&handoff->condition, &handoff->mutex);
    }
    handoff->value *= 2;
    (void)pthread_mutex_unlock(&handoff->mutex);
    return NULL;
}

int main(void)
{
    struct handoff handoff = {
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_COND_INITIALIZER,
        0,
        0,
    };
    pthread_t thread;
    int result;

    result = pthread_create(&thread, NULL, consume_value, &handoff);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        return EXIT_FAILURE;
    }

    (void)pthread_mutex_lock(&handoff.mutex);
    handoff.value = 21;
    handoff.ready = 1;
    (void)pthread_cond_signal(&handoff.condition);
    (void)pthread_mutex_unlock(&handoff.mutex);

    result = pthread_join(thread, NULL);
    if (result != 0) {
        report_pthread_error("pthread_join", result);
        return EXIT_FAILURE;
    }

    printf("ready=%d transformed_value=%d\n", handoff.ready, handoff.value);
    printf("predicate_loop_used=yes handoff_completed=%s\n",
           handoff.value == 42 ? "yes" : "no");

    (void)pthread_cond_destroy(&handoff.condition);
    (void)pthread_mutex_destroy(&handoff.mutex);
    return handoff.value == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}
