/*
 * Experiment - Thread-per-task pattern
 *
 * Prediction:
 *   Assigning one independent task to each thread is easy to reason about,
 *   but the number of live threads grows with the number of simultaneous
 *   tasks.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

enum { TASK_COUNT = 6 };

struct task {
    int input;
    int output;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *run_task(void *argument)
{
    struct task *task = argument;

    task->output = task->input * task->input;
    return NULL;
}

int main(void)
{
    struct task tasks[TASK_COUNT];
    pthread_t threads[TASK_COUNT];
    int sum = 0;
    int result;

    for (int index = 0; index < TASK_COUNT; ++index) {
        tasks[index].input = index + 1;
        tasks[index].output = 0;
        result = pthread_create(&threads[index], NULL, run_task, &tasks[index]);
        if (result != 0) {
            report_pthread_error("pthread_create", result);
            return EXIT_FAILURE;
        }
    }
    for (int index = 0; index < TASK_COUNT; ++index) {
        result = pthread_join(threads[index], NULL);
        if (result != 0) {
            report_pthread_error("pthread_join", result);
            return EXIT_FAILURE;
        }
        sum += tasks[index].output;
    }

    printf("tasks=%d threads=%d sum_of_squares=%d\n", TASK_COUNT, TASK_COUNT, sum);
    printf("thread_per_task_completed=%s scaling_cost=one-thread-per-live-task\n",
           sum == 91 ? "yes" : "no");
    return sum == 91 ? EXIT_SUCCESS : EXIT_FAILURE;
}
