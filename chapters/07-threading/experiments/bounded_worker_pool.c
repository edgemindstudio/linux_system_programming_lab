/*
 * Experiment - Reuse a bounded set of workers
 *
 * Prediction:
 *   A fixed worker pool can process more tasks than there are threads. A mutex
 *   protects allocation of the next task while work occurs outside the lock.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

enum { WORKER_COUNT = 2, TASK_COUNT = 8 };

struct work_queue {
    pthread_mutex_t mutex;
    int next_task;
    int results[TASK_COUNT];
    int processed[WORKER_COUNT];
};

struct worker_argument {
    struct work_queue *queue;
    int worker_index;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *pool_worker(void *argument)
{
    struct worker_argument *worker = argument;

    for (;;) {
        int task_index;

        (void)pthread_mutex_lock(&worker->queue->mutex);
        task_index = worker->queue->next_task;
        if (task_index < TASK_COUNT) {
            ++worker->queue->next_task;
        }
        (void)pthread_mutex_unlock(&worker->queue->mutex);

        if (task_index >= TASK_COUNT) {
            break;
        }
        worker->queue->results[task_index] = (task_index + 1) * (task_index + 1);
        ++worker->queue->processed[worker->worker_index];
    }
    return NULL;
}

int main(void)
{
    struct work_queue queue = {
        PTHREAD_MUTEX_INITIALIZER,
        0,
        {0},
        {0},
    };
    struct worker_argument arguments[WORKER_COUNT];
    pthread_t workers[WORKER_COUNT];
    int result;
    int sum = 0;
    int processed_total;

    for (int index = 0; index < WORKER_COUNT; ++index) {
        arguments[index].queue = &queue;
        arguments[index].worker_index = index;
        result = pthread_create(&workers[index], NULL, pool_worker, &arguments[index]);
        if (result != 0) {
            report_pthread_error("pthread_create", result);
            return EXIT_FAILURE;
        }
    }
    for (int index = 0; index < WORKER_COUNT; ++index) {
        result = pthread_join(workers[index], NULL);
        if (result != 0) {
            report_pthread_error("pthread_join", result);
            return EXIT_FAILURE;
        }
    }

    for (int index = 0; index < TASK_COUNT; ++index) {
        sum += queue.results[index];
    }
    processed_total = queue.processed[0] + queue.processed[1];
    printf("workers=%d tasks=%d processed=%d sum_of_squares=%d\n",
           WORKER_COUNT,
           TASK_COUNT,
           processed_total,
           sum);
    printf("workers_reused=%s bounded_thread_count=yes\n",
           processed_total == TASK_COUNT && sum == 204 ? "yes" : "no");

    (void)pthread_mutex_destroy(&queue.mutex);
    return processed_total == TASK_COUNT && sum == 204
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
