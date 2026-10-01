/*
 * Exercise 07.05 - Distinguish shared memory from per-thread stacks
 *
 * Purpose:
 *   Show that threads share the process address space but execute with
 *   separate stacks and therefore separate automatic local variables.
 *
 * Linux behavior:
 *   Each worker records the address of its own local object into a distinct
 *   result slot. The addresses should be nonzero and different.
 */

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct stack_report {
    uintptr_t local_address;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *record_stack_address(void *argument)
{
    struct stack_report *report = argument;
    int local_marker = 0;

    report->local_address = (uintptr_t)&local_marker;
    return NULL;
}

int main(void)
{
    pthread_t first;
    pthread_t second;
    struct stack_report reports[2] = {{0U}, {0U}};
    int result;

    result = pthread_create(&first, NULL, record_stack_address, &reports[0]);
    if (result != 0) {
        report_pthread_error("pthread_create first", result);
        return EXIT_FAILURE;
    }
    result = pthread_create(&second, NULL, record_stack_address, &reports[1]);
    if (result != 0) {
        report_pthread_error("pthread_create second", result);
        (void)pthread_join(first, NULL);
        return EXIT_FAILURE;
    }

    result = pthread_join(first, NULL);
    if (result != 0) {
        report_pthread_error("pthread_join first", result);
        return EXIT_FAILURE;
    }
    result = pthread_join(second, NULL);
    if (result != 0) {
        report_pthread_error("pthread_join second", result);
        return EXIT_FAILURE;
    }

    printf("addresses_nonzero=%s stack_addresses_distinct=%s\n",
           reports[0].local_address != 0U && reports[1].local_address != 0U
               ? "yes"
               : "no",
           reports[0].local_address != reports[1].local_address ? "yes" : "no");
    printf("memory_model=shared-address-space-with-private-stacks\n");

    return EXIT_SUCCESS;
}
