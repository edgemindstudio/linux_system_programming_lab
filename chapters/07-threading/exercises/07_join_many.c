/*
 * Exercise 07.07 - Join several peer threads
 *
 * Purpose:
 *   Create multiple workers and reclaim every joinable thread explicitly.
 *
 * Linux behavior:
 *   Pthreads are peers rather than parent/child objects. The main thread can
 *   join each worker once and then safely consume every completed result.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

enum { WORKER_COUNT = 4 };

struct work_item {
    int input;
    int output;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *triple_value(void *argument)
{
    struct work_item *item = argument;

    item->output = item->input * 3;
    return NULL;
}

int main(void)
{
    pthread_t threads[WORKER_COUNT];
    struct work_item items[WORKER_COUNT] = {{1, 0}, {2, 0}, {3, 0}, {4, 0}};
    int total = 0;
    int created = 0;
    int result;

    for (int index = 0; index < WORKER_COUNT; ++index) {
        result = pthread_create(&threads[index], NULL, triple_value, &items[index]);
        if (result != 0) {
            report_pthread_error("pthread_create", result);
            break;
        }
        ++created;
    }

    for (int index = 0; index < created; ++index) {
        result = pthread_join(threads[index], NULL);
        if (result != 0) {
            report_pthread_error("pthread_join", result);
            return EXIT_FAILURE;
        }
        total += items[index].output;
    }

    printf("threads_created=%d threads_joined=%d total=%d\n",
           created,
           created,
           total);
    printf("all_resources_reclaimed=%s results_complete=%s\n",
           created == WORKER_COUNT ? "yes" : "no",
           total == 30 ? "yes" : "no");

    return created == WORKER_COUNT && total == 30 ? EXIT_SUCCESS : EXIT_FAILURE;
}
