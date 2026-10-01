/*
 * Exercise 07.11 - Request deferred cancellation
 *
 * Purpose:
 *   Cancel a worker at an explicit cancellation point and verify the special
 *   PTHREAD_CANCELED result returned by pthread_join().
 *
 * Linux behavior:
 *   pthread_cancel() submits a request; it does not synchronously destroy the
 *   target. With deferred cancellation, termination occurs only when the
 *   worker reaches a cancellation point such as pthread_testcancel().
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct cancellation_state {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int ready;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *cancellable_worker(void *argument)
{
    struct cancellation_state *state = argument;
    int old_state;
    int old_type;

    (void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);
    (void)pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old_type);

    (void)pthread_mutex_lock(&state->mutex);
    state->ready = 1;
    (void)pthread_cond_signal(&state->condition);
    (void)pthread_mutex_unlock(&state->mutex);

    for (;;) {
        pthread_testcancel();
    }
    return NULL;
}

int main(void)
{
    struct cancellation_state state = {
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_COND_INITIALIZER,
        0,
    };
    pthread_t thread;
    void *returned = NULL;
    int result;

    result = pthread_create(&thread, NULL, cancellable_worker, &state);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        return EXIT_FAILURE;
    }

    (void)pthread_mutex_lock(&state.mutex);
    while (state.ready == 0) {
        (void)pthread_cond_wait(&state.condition, &state.mutex);
    }
    (void)pthread_mutex_unlock(&state.mutex);

    result = pthread_cancel(thread);
    if (result != 0) {
        report_pthread_error("pthread_cancel", result);
        return EXIT_FAILURE;
    }
    result = pthread_join(thread, &returned);
    if (result != 0) {
        report_pthread_error("pthread_join", result);
        return EXIT_FAILURE;
    }

    printf("cancel_request_accepted=yes joined=yes\n");
    printf("deferred_cancellation_observed=%s\n",
           returned == PTHREAD_CANCELED ? "yes" : "no");

    (void)pthread_cond_destroy(&state.condition);
    (void)pthread_mutex_destroy(&state.mutex);
    return returned == PTHREAD_CANCELED ? EXIT_SUCCESS : EXIT_FAILURE;
}
