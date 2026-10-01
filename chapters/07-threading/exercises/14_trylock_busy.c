/*
 * Exercise 07.14 - Observe a busy mutex without blocking
 *
 * Purpose:
 *   Use pthread_mutex_trylock() when a thread must not wait for a lock.
 *
 * Linux behavior:
 *   Main holds the mutex before creating the worker. The worker therefore
 *   receives EBUSY immediately rather than blocking inside mutex acquisition.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct trylock_report {
    pthread_mutex_t *mutex;
    int result;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *try_locked_mutex(void *argument)
{
    struct trylock_report *report = argument;

    report->result = pthread_mutex_trylock(report->mutex);
    if (report->result == 0) {
        (void)pthread_mutex_unlock(report->mutex);
    }
    return NULL;
}

int main(void)
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    struct trylock_report report = {&mutex, 0};
    pthread_t thread;
    int result;

    result = pthread_mutex_lock(&mutex);
    if (result != 0) {
        report_pthread_error("pthread_mutex_lock", result);
        return EXIT_FAILURE;
    }
    result = pthread_create(&thread, NULL, try_locked_mutex, &report);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        (void)pthread_mutex_unlock(&mutex);
        return EXIT_FAILURE;
    }
    result = pthread_join(thread, NULL);
    if (result != 0) {
        report_pthread_error("pthread_join", result);
        (void)pthread_mutex_unlock(&mutex);
        return EXIT_FAILURE;
    }
    (void)pthread_mutex_unlock(&mutex);

    printf("trylock_result=%d expected_ebusy=%d\n", report.result, EBUSY);
    printf("busy_observed=%s worker_blocked=no\n",
           report.result == EBUSY ? "yes" : "no");

    (void)pthread_mutex_destroy(&mutex);
    return report.result == EBUSY ? EXIT_SUCCESS : EXIT_FAILURE;
}
