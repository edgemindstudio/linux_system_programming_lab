/*
 * Experiment - Observe Linux tasks through /proc/self/task
 *
 * Prediction:
 *   While four workers are alive, /proc/self/task contains at least five task
 *   directories: one for the initial thread and one for each worker.
 *
 * Portability:
 *   /proc/self/task is Linux-specific evidence, not part of POSIX Pthreads.
 */

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

enum { WORKER_COUNT = 4 };

struct gate {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int ready_count;
    int release;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *wait_at_gate(void *argument)
{
    struct gate *gate = argument;

    (void)pthread_mutex_lock(&gate->mutex);
    ++gate->ready_count;
    (void)pthread_cond_broadcast(&gate->condition);
    while (gate->release == 0) {
        (void)pthread_cond_wait(&gate->condition, &gate->mutex);
    }
    (void)pthread_mutex_unlock(&gate->mutex);
    return NULL;
}

static int count_tasks(void)
{
    DIR *directory = opendir("/proc/self/task");
    struct dirent *entry;
    int count = 0;

    if (directory == NULL) {
        return -1;
    }
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_name[0] != '.') {
            ++count;
        }
    }
    (void)closedir(directory);
    return count;
}

int main(void)
{
    struct gate gate = {
        PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_COND_INITIALIZER,
        0,
        0,
    };
    pthread_t threads[WORKER_COUNT];
    int observed_tasks;
    int result;

    for (int index = 0; index < WORKER_COUNT; ++index) {
        result = pthread_create(&threads[index], NULL, wait_at_gate, &gate);
        if (result != 0) {
            report_pthread_error("pthread_create", result);
            return EXIT_FAILURE;
        }
    }

    (void)pthread_mutex_lock(&gate.mutex);
    while (gate.ready_count != WORKER_COUNT) {
        (void)pthread_cond_wait(&gate.condition, &gate.mutex);
    }
    observed_tasks = count_tasks();
    gate.release = 1;
    (void)pthread_cond_broadcast(&gate.condition);
    (void)pthread_mutex_unlock(&gate.mutex);

    for (int index = 0; index < WORKER_COUNT; ++index) {
        (void)pthread_join(threads[index], NULL);
    }

    printf("workers=%d observed_proc_tasks=%d\n", WORKER_COUNT, observed_tasks);
    printf("initial_plus_workers_visible=%s linux_specific_interface=yes\n",
           observed_tasks >= WORKER_COUNT + 1 ? "yes" : "no");

    (void)pthread_cond_destroy(&gate.condition);
    (void)pthread_mutex_destroy(&gate.mutex);
    return observed_tasks >= WORKER_COUNT + 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
