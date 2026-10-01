/*
 * Exercise 07.02 - Create and join one thread
 *
 * Purpose:
 *   Practice the smallest complete Pthreads lifecycle: create a thread, let
 *   it execute a start routine, and join it before process termination.
 *
 * Linux behavior:
 *   pthread_create() returns an error number directly. pthread_join() waits
 *   for a joinable thread and establishes that its writes are visible to the
 *   joining thread.
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

static void *compute_answer(void *argument)
{
    int *answer = argument;

    *answer = 6 * 7;
    return argument;
}

int main(void)
{
    pthread_t thread;
    int answer = 0;
    void *returned = NULL;
    int result;

    result = pthread_create(&thread, NULL, compute_answer, &answer);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        return EXIT_FAILURE;
    }

    result = pthread_join(thread, &returned);
    if (result != 0) {
        report_pthread_error("pthread_join", result);
        return EXIT_FAILURE;
    }

    printf("answer=%d returned_same_address=%s\n",
           answer,
           returned == &answer ? "yes" : "no");
    printf("thread_created=yes thread_joined=yes result_visible=%s\n",
           answer == 42 ? "yes" : "no");

    return answer == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}
