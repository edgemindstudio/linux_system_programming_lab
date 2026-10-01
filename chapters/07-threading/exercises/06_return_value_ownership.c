/*
 * Exercise 07.06 - Transfer ownership through pthread_join()
 *
 * Purpose:
 *   Return a dynamically allocated result from a worker and make the joining
 *   thread responsible for releasing that result.
 *
 * Linux behavior:
 *   The pointer returned from the start routine is delivered through the
 *   second argument of pthread_join(). The allocation remains process memory;
 *   terminating the worker does not free it automatically.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *allocate_result(void *argument)
{
    const int input = *(const int *)argument;
    int *result = malloc(sizeof(*result));

    if (result != NULL) {
        *result = input * 2;
    }
    return result;
}

int main(void)
{
    pthread_t thread;
    int input = 21;
    void *returned = NULL;
    int *answer;
    int result;

    result = pthread_create(&thread, NULL, allocate_result, &input);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        return EXIT_FAILURE;
    }
    result = pthread_join(thread, &returned);
    if (result != 0) {
        report_pthread_error("pthread_join", result);
        return EXIT_FAILURE;
    }

    answer = returned;
    if (answer == NULL) {
        fputs("worker allocation failed\n", stderr);
        return EXIT_FAILURE;
    }

    printf("worker_result=%d ownership_transferred=yes\n", *answer);
    printf("joining_thread_frees_result=yes value_correct=%s\n",
           *answer == 42 ? "yes" : "no");
    free(answer);

    return EXIT_SUCCESS;
}
