/*
 * Exercise 07.09 - Terminate the initial thread without ending the process
 *
 * Purpose:
 *   Contrast returning from main(), which terminates the process, with calling
 *   pthread_exit(), which terminates only the calling thread.
 *
 * Linux behavior:
 *   The process remains alive while the worker exists. The worker therefore
 *   reaches its start routine even after the initial thread has exited.
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

static void *last_worker(void *argument)
{
    const char *message = argument;

    printf("%s\n", message);
    printf("worker_completed_after_initial_thread_exit=yes\n");
    return NULL;
}

int main(void)
{
    static const char message[] = "worker_observed=running";
    pthread_t thread;
    int result = pthread_create(&thread, NULL, last_worker, (void *)message);

    if (result != 0) {
        report_pthread_error("pthread_create", result);
        return EXIT_FAILURE;
    }

    puts("initial_thread_action=pthread_exit");
    fflush(stdout);
    pthread_exit(NULL);
}
