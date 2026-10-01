/*
 * Exercise 07.08 - Detach a thread without losing completion coordination
 *
 * Purpose:
 *   Release a thread's join resources automatically while using a condition
 *   variable to learn when its application-level work is complete.
 *
 * Linux behavior:
 *   pthread_detach() makes pthread_join() inappropriate for that thread. It
 *   does not provide a completion notification, so the program needs a
 *   separate synchronization protocol before destroying shared state.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct completion {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int done;
    int value;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *detached_worker(void *argument)
{
    struct completion *completion = argument;

    (void)pthread_mutex_lock(&completion->mutex);
    completion->value = 42;
    completion->done = 1;
    (void)pthread_cond_signal(&completion->condition);
    (void)pthread_mutex_unlock(&completion->mutex);
    return NULL;
}

int main(void)
{
    struct completion completion = {
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_COND_INITIALIZER,
        0,
        0,
    };
    pthread_t thread;
    int result;

    result = pthread_create(&thread, NULL, detached_worker, &completion);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        return EXIT_FAILURE;
    }
    result = pthread_detach(thread);
    if (result != 0) {
        report_pthread_error("pthread_detach", result);
        return EXIT_FAILURE;
    }

    result = pthread_mutex_lock(&completion.mutex);
    if (result != 0) {
        report_pthread_error("pthread_mutex_lock", result);
        return EXIT_FAILURE;
    }
    while (completion.done == 0) {
        result = pthread_cond_wait(&completion.condition, &completion.mutex);
        if (result != 0) {
            report_pthread_error("pthread_cond_wait", result);
            (void)pthread_mutex_unlock(&completion.mutex);
            return EXIT_FAILURE;
        }
    }
    result = pthread_mutex_unlock(&completion.mutex);
    if (result != 0) {
        report_pthread_error("pthread_mutex_unlock", result);
        return EXIT_FAILURE;
    }

    printf("detached=yes completion_observed=yes value=%d\n", completion.value);
    printf("join_attempted=no separate_protocol_used=yes\n");

    (void)pthread_cond_destroy(&completion.condition);
    (void)pthread_mutex_destroy(&completion.mutex);
    return completion.value == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}
