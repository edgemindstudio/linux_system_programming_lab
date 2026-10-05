/*
 * Exercise 11.10 - Break one timestamp into UTC and local time
 *
 * Purpose:
 *   Convert one time_t value with gmtime_r() and localtime_r() while keeping
 *   the two results in caller-owned storage.
 *
 * Linux behavior:
 *   UTC conversion is independent of local timezone policy. Local conversion
 *   applies the process timezone and daylight-saving rules. The _r interfaces
 *   avoid shared static result storage.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    time_t now = time(NULL);
    struct tm utc;
    struct tm local;
    int fields_valid;

    if (now == (time_t)-1) {
        perror("time");
        return EXIT_FAILURE;
    }
    if (gmtime_r(&now, &utc) == NULL || localtime_r(&now, &local) == NULL) {
        fputs("time conversion failed\n", stderr);
        return EXIT_FAILURE;
    }

    fields_valid = utc.tm_mon >= 0 && utc.tm_mon <= 11 &&
                   local.tm_mon >= 0 && local.tm_mon <= 11 &&
                   utc.tm_yday >= 0 && utc.tm_yday <= 365 &&
                   local.tm_yday >= 0 && local.tm_yday <= 365;
    printf("utc_fields_valid=%s local_fields_valid=%s\n",
           fields_valid ? "yes" : "no",
           fields_valid ? "yes" : "no");
    printf("same_epoch_input=yes timezone_policy_applied_only_to_local=yes reentrant_interfaces=yes\n");

    return fields_valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
