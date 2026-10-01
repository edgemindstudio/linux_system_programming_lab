/*
 * Exercise 07.17 - Detect a self-deadlock during development
 *
 * Purpose:
 *   Configure an error-checking mutex and observe EDEADLK when the same thread
 *   attempts to lock it twice.
 *
 * Linux behavior:
 *   A default mutex may simply deadlock on recursive acquisition. The
 *   ERRORCHECK type converts this particular bug into a diagnosable error.
 */

#define _XOPEN_SOURCE 700

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

int main(void)
{
    pthread_mutexattr_t attributes;
    pthread_mutex_t mutex;
    int first_lock;
    int second_lock;
    int result;

    result = pthread_mutexattr_init(&attributes);
    if (result != 0) {
        report_pthread_error("pthread_mutexattr_init", result);
        return EXIT_FAILURE;
    }
    result = pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_ERRORCHECK);
    if (result != 0) {
        report_pthread_error("pthread_mutexattr_settype", result);
        return EXIT_FAILURE;
    }
    result = pthread_mutex_init(&mutex, &attributes);
    if (result != 0) {
        report_pthread_error("pthread_mutex_init", result);
        return EXIT_FAILURE;
    }
    (void)pthread_mutexattr_destroy(&attributes);

    first_lock = pthread_mutex_lock(&mutex);
    second_lock = pthread_mutex_lock(&mutex);
    if (first_lock == 0) {
        (void)pthread_mutex_unlock(&mutex);
    }

    printf("first_lock=%d second_lock=%d expected_edeadlk=%d\n",
           first_lock,
           second_lock,
           EDEADLK);
    printf("self_deadlock_detected=%s program_hung=no\n",
           first_lock == 0 && second_lock == EDEADLK ? "yes" : "no");

    (void)pthread_mutex_destroy(&mutex);
    return first_lock == 0 && second_lock == EDEADLK
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
