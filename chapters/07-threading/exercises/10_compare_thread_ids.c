/*
 * Exercise 07.10 - Compare opaque pthread_t values correctly
 *
 * Purpose:
 *   Use pthread_equal() instead of assuming pthread_t is an integer suitable
 *   for arithmetic comparison or formatted output.
 *
 * Linux behavior:
 *   The handle returned by pthread_create() represents the same Pthread as
 *   pthread_self() inside the worker, but differs from the main thread's ID.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct id_comparison {
    pthread_t main_id;
    pthread_t worker_self;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *capture_self(void *argument)
{
    struct id_comparison *comparison = argument;

    comparison->worker_self = pthread_self();
    return NULL;
}

int main(void)
{
    struct id_comparison comparison;
    pthread_t created;
    int result;

    comparison.main_id = pthread_self();
    result = pthread_create(&created, NULL, capture_self, &comparison);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        return EXIT_FAILURE;
    }
    result = pthread_join(created, NULL);
    if (result != 0) {
        report_pthread_error("pthread_join", result);
        return EXIT_FAILURE;
    }

    printf("created_matches_worker_self=%s\n",
           pthread_equal(created, comparison.worker_self) != 0 ? "yes" : "no");
    printf("main_differs_from_worker=%s opaque_ids_compared_portably=yes\n",
           pthread_equal(comparison.main_id, comparison.worker_self) == 0
               ? "yes"
               : "no");

    return EXIT_SUCCESS;
}
