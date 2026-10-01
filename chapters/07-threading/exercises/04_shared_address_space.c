/*
 * Exercise 07.04 - Observe the shared process address space
 *
 * Purpose:
 *   Show that threads do not receive copy-on-write memory as forked processes
 *   do. They access the same objects in one virtual address space.
 *
 * Linux behavior:
 *   The worker updates an object owned by main. pthread_join() supplies the
 *   required synchronization before main reads the result.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct shared_record {
    int value;
    int updated_by_worker;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *update_record(void *argument)
{
    struct shared_record *record = argument;

    record->value = 73;
    record->updated_by_worker = 1;
    return NULL;
}

int main(void)
{
    struct shared_record record = {10, 0};
    pthread_t thread;
    int result;

    result = pthread_create(&thread, NULL, update_record, &record);
    if (result != 0) {
        report_pthread_error("pthread_create", result);
        return EXIT_FAILURE;
    }
    result = pthread_join(thread, NULL);
    if (result != 0) {
        report_pthread_error("pthread_join", result);
        return EXIT_FAILURE;
    }

    printf("value_after_join=%d updated_by_worker=%d\n",
           record.value,
           record.updated_by_worker);
    printf("address_space_shared=%s join_synchronized=yes\n",
           record.value == 73 && record.updated_by_worker == 1 ? "yes" : "no");

    return EXIT_SUCCESS;
}
