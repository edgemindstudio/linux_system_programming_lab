/*
 * Exercise 07.15 - Lock data, not merely a function
 *
 * Purpose:
 *   Attach a mutex to an account object and preserve the invariant that
 *   successful withdrawals cannot overdraw its balance.
 *
 * Linux behavior:
 *   Two workers request a combined amount larger than the balance. Locking
 *   the account across check-and-update makes exactly one request succeed.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct account {
    pthread_mutex_t mutex;
    int balance;
};

struct withdrawal {
    struct account *account;
    int amount;
    int succeeded;
};

static void report_pthread_error(const char *operation, int error_code)
{
    errno = error_code;
    perror(operation);
}

static void *withdraw(void *argument)
{
    struct withdrawal *request = argument;

    (void)pthread_mutex_lock(&request->account->mutex);
    if (request->account->balance >= request->amount) {
        request->account->balance -= request->amount;
        request->succeeded = 1;
    }
    (void)pthread_mutex_unlock(&request->account->mutex);
    return NULL;
}

int main(void)
{
    struct account account = {PTHREAD_MUTEX_INITIALIZER, 500};
    struct withdrawal requests[2] = {{&account, 400, 0}, {&account, 200, 0}};
    pthread_t threads[2];
    int result;
    int succeeded;
    int successful_amount;

    for (int index = 0; index < 2; ++index) {
        result = pthread_create(&threads[index], NULL, withdraw, &requests[index]);
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

    succeeded = requests[0].succeeded + requests[1].succeeded;
    successful_amount = requests[0].succeeded != 0 ? requests[0].amount
                                                    : requests[1].amount;
    printf("successful_withdrawals=%d final_balance=%d\n",
           succeeded,
           account.balance);
    printf("one_succeeded=%s no_overdraft=%s invariant_preserved=%s\n",
           succeeded == 1 ? "yes" : "no",
           account.balance >= 0 ? "yes" : "no",
           account.balance == 500 - successful_amount ? "yes" : "no");

    (void)pthread_mutex_destroy(&account.mutex);
    return succeeded == 1 && account.balance >= 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
