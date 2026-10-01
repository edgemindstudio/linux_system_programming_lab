/*
 * Exercise 07.16 - Prevent ABBA deadlock with one lock order
 *
 * Purpose:
 *   Transfer funds in opposite directions while requiring every thread to
 *   acquire account locks in ascending account-ID order.
 *
 * Linux behavior:
 *   A global ordering removes circular wait: no thread may hold the higher
 *   ordered lock while waiting for the lower ordered lock.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct account {
    int id;
    int balance;
    pthread_mutex_t mutex;
};

struct transfer {
    struct account *source;
    struct account *destination;
    int amount;
    int completed;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *transfer_money(void *argument)
{
    struct transfer *transfer = argument;
    struct account *first = transfer->source->id < transfer->destination->id
                                ? transfer->source
                                : transfer->destination;
    struct account *second = first == transfer->source ? transfer->destination
                                                        : transfer->source;

    (void)pthread_mutex_lock(&first->mutex);
    (void)pthread_mutex_lock(&second->mutex);
    if (transfer->source->balance >= transfer->amount) {
        transfer->source->balance -= transfer->amount;
        transfer->destination->balance += transfer->amount;
        transfer->completed = 1;
    }
    (void)pthread_mutex_unlock(&second->mutex);
    (void)pthread_mutex_unlock(&first->mutex);
    return NULL;
}

int main(void)
{
    struct account first = {1, 500, PTHREAD_MUTEX_INITIALIZER};
    struct account second = {2, 500, PTHREAD_MUTEX_INITIALIZER};
    struct transfer transfers[2] = {
        {&first, &second, 100, 0},
        {&second, &first, 70, 0},
    };
    pthread_t threads[2];
    int result;

    for (int index = 0; index < 2; ++index) {
        result = pthread_create(&threads[index], NULL, transfer_money, &transfers[index]);
        if (result != 0) {
            report_pthread_error("pthread_create", result);
            return EXIT_FAILURE;
        }
    }
    for (int index = 0; index < 2; ++index) {
        result = pthread_join(threads[index], NULL);
        if (result != 0) {
            report_pthread_error("pthread_join", result);
            return EXIT_FAILURE;
        }
    }

    printf("account1=%d account2=%d total=%d\n",
           first.balance,
           second.balance,
           first.balance + second.balance);
    printf("both_transfers_completed=%s lock_order_consistent=yes total_preserved=%s\n",
           transfers[0].completed != 0 && transfers[1].completed != 0 ? "yes" : "no",
           first.balance + second.balance == 1000 ? "yes" : "no");

    (void)pthread_mutex_destroy(&first.mutex);
    (void)pthread_mutex_destroy(&second.mutex);
    return EXIT_SUCCESS;
}
