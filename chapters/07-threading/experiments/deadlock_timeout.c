/*
 * Experiment - Turn indefinite waiting into a bounded observation
 *
 * Prediction:
 *   If main holds a mutex, a worker's timed lock attempt reaches ETIMEDOUT
 *   instead of hanging the laboratory forever.
 *
 * Lesson:
 *   Timeouts help diagnosis and recovery, but they do not replace a designed
 *   lock hierarchy. Consistent ordering is the primary deadlock prevention.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct timed_attempt {
    pthread_mutex_t *mutex;
    int result;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *attempt_timed_lock(void *argument)
{
    struct timed_attempt *attempt = argument;
    struct timespec deadline;

    if (clock_gettime(CLOCK_REALTIME, &deadline) == -1) {
        attempt->result = errno;
        return NULL;
    }
    deadline.tv_nsec += 100000000L;
    if (deadline.tv_nsec >= 1000000000L) {
        ++deadline.tv_sec;
        deadline.tv_nsec -= 1000000000L;
    }

    attempt->result = pthread_mutex_timedlock(attempt->mutex, &deadline);
    if (attempt->result == 0) {
        (void)pthread_mutex_unlock(attempt->mutex);
    }
    return NULL;
}

int main(void)
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    struct timed_attempt attempt = {&mutex, 0};
    pthread_t worker;
    int result;

    result = pthread_mutex_lock(&mutex);
    if (result != 0) {
        report_pthread_error("pthread_mutex_lock", result);
        return EXIT_FAILURE;
    }
    result = pthread_create(&worker, NULL, attempt_timed_lock, &attempt);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        (void)pthread_mutex_unlock(&mutex);
        return EXIT_FAILURE;
    }
    result = pthread_join(worker, NULL);
    if (result != 0) {
        report_pthread_error("pthread_join", result);
        (void)pthread_mutex_unlock(&mutex);
        return EXIT_FAILURE;
    }
    (void)pthread_mutex_unlock(&mutex);

    printf("timed_lock_result=%d expected_timeout=%d\n", attempt.result, ETIMEDOUT);
    printf("wait_bounded=%s process_hung=no ordering_still_required=yes\n",
           attempt.result == ETIMEDOUT ? "yes" : "no");

    (void)pthread_mutex_destroy(&mutex);
    return attempt.result == ETIMEDOUT ? EXIT_SUCCESS : EXIT_FAILURE;
}
