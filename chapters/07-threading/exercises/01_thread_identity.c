/*
 * Exercise 07.01 - Process identity and thread identity
 *
 * Purpose:
 *   Separate the identity shared by every thread in a process from the
 *   identities assigned to individual units of execution.
 *
 * Linux behavior:
 *   getpid() returns the same process ID in both threads. Linux also gives
 *   each thread a kernel task ID, while Pthreads exposes an opaque pthread_t.
 *   Portable code compares pthread_t values with pthread_equal().
 */

#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

struct identity_report {
    pid_t process_id;
    pid_t kernel_tid;
    pthread_t pthread_id;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *inspect_identity(void *argument)
{
    struct identity_report *report = argument;

    report->process_id = getpid();
    report->kernel_tid = (pid_t)syscall(SYS_gettid);
    report->pthread_id = pthread_self();
    return NULL;
}

int main(void)
{
    struct identity_report worker = {0};
    pthread_t thread;
    pthread_t main_pthread_id = pthread_self();
    pid_t main_process_id = getpid();
    pid_t main_kernel_tid = (pid_t)syscall(SYS_gettid);
    int result;

    result = pthread_create(&thread, NULL, inspect_identity, &worker);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        return EXIT_FAILURE;
    }

    result = pthread_join(thread, NULL);
    if (result != 0) {
        report_pthread_error("pthread_join", result);
        return EXIT_FAILURE;
    }

    printf("main_pid=%ld worker_pid=%ld main_tid=%ld worker_tid=%ld\n",
           (long)main_process_id,
           (long)worker.process_id,
           (long)main_kernel_tid,
           (long)worker.kernel_tid);
    printf("process_shared=%s kernel_tasks_distinct=%s pthread_ids_distinct=%s\n",
           main_process_id == worker.process_id ? "yes" : "no",
           main_kernel_tid != worker.kernel_tid ? "yes" : "no",
           pthread_equal(main_pthread_id, worker.pthread_id) == 0 ? "yes" : "no");

    return EXIT_SUCCESS;
}
