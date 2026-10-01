/*
 * Exercise 07.12 - Release resources during cancellation
 *
 * Purpose:
 *   Register a cleanup handler so deferred cancellation does not leak a
 *   worker-owned allocation.
 *
 * Linux behavior:
 *   Cancellation cleanup handlers execute in last-in, first-out order while
 *   the target thread is unwound. They are essential when cancellation can
 *   occur after resources have been acquired.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct cleanup_state {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int ready;
    int cleanup_called;
};

struct cleanup_resource {
    struct cleanup_state *state;
    void *allocation;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void release_resource(void *argument)
{
    struct cleanup_resource *resource = argument;

    free(resource->allocation);
    resource->state->cleanup_called = 1;
}

static void *worker_with_cleanup(void *argument)
{
    struct cleanup_state *state = argument;
    struct cleanup_resource resource = {state, malloc(256U)};

    if (resource.allocation == NULL) {
        return NULL;
    }

    pthread_cleanup_push(release_resource, &resource);
    (void)pthread_mutex_lock(&state->mutex);
    state->ready = 1;
    (void)pthread_cond_signal(&state->condition);
    (void)pthread_mutex_unlock(&state->mutex);

    for (;;) {
        pthread_testcancel();
    }
    pthread_cleanup_pop(1);
    return NULL;
}

int main(void)
{
    struct cleanup_state state = {
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_COND_INITIALIZER,
        0,
        0,
    };
    pthread_t thread;
    void *returned = NULL;
    int result;

    result = pthread_create(&thread, NULL, worker_with_cleanup, &state);
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

    printf("thread_canceled=%s cleanup_called=%s\n",
           returned == PTHREAD_CANCELED ? "yes" : "no",
           state.cleanup_called != 0 ? "yes" : "no");
    printf("worker_allocation_released=%s\n",
           state.cleanup_called != 0 ? "yes" : "no");

    (void)pthread_cond_destroy(&state.condition);
    (void)pthread_mutex_destroy(&state.mutex);
    return state.cleanup_called != 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
