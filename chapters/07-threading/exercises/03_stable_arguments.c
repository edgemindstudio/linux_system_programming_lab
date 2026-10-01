/*
 * Exercise 07.03 - Give each thread stable argument storage
 *
 * Purpose:
 *   Pass structured input to several threads without sharing the address of
 *   a changing loop variable.
 *
 * Linux behavior:
 *   pthread_create() passes only a void pointer. The pointed-to object must
 *   remain alive until the worker has finished using it. One array element
 *   per thread gives every worker stable, distinct storage.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

enum { THREAD_COUNT = 3 };

struct task {
    int input;
    int square;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *square_input(void *argument)
{
    struct task *task = argument;

    task->square = task->input * task->input;
    return NULL;
}

int main(void)
{
    pthread_t threads[THREAD_COUNT];
    struct task tasks[THREAD_COUNT] = {{2, 0}, {3, 0}, {4, 0}};
    int created = 0;
    int result;

    for (int index = 0; index < THREAD_COUNT; ++index) {
        result = pthread_create(&threads[index], NULL, square_input, &tasks[index]);
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
    }

    if (created != THREAD_COUNT) {
        return EXIT_FAILURE;
    }

    printf("squares=%d,%d,%d\n", tasks[0].square, tasks[1].square, tasks[2].square);
    printf("arguments_stable=%s all_threads_joined=yes\n",
           tasks[0].square == 4 && tasks[1].square == 9 && tasks[2].square == 16
               ? "yes"
               : "no");

    return EXIT_SUCCESS;
}
