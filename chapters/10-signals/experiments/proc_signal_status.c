/*
 * Experiment - Compare configured signal state with /proc/self/status
 *
 * Prediction:
 *   Linux exposes hexadecimal masks for blocked, ignored, and caught signals.
 *   After configuration, the corresponding SIGTERM, SIGUSR1, and SIGUSR2 bits
 *   should be visible in SigBlk, SigIgn, and SigCgt.
 *
 * Observation boundary:
 *   /proc is a Linux diagnostic interface. Its text format is useful evidence,
 *   not a portable replacement for sigaction() and sigprocmask().
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void unused_handler(int signal_number)
{
    (void)signal_number;
}

static int mask_contains(unsigned long long mask, int signal_number)
{
    unsigned int bit = (unsigned int)(signal_number - 1);

    if (bit >= 8U * (unsigned int)sizeof(mask)) {
        return 0;
    }
    return (mask & (1ULL << bit)) != 0ULL;
}

int main(void)
{
    struct sigaction ignored = {0};
    struct sigaction caught = {0};
    sigset_t block_set;
    sigset_t previous_mask;
    FILE *status_file;
    char *line = NULL;
    size_t capacity = 0U;
    unsigned long long blocked_mask = 0ULL;
    unsigned long long ignored_mask = 0ULL;
    unsigned long long caught_mask = 0ULL;
    int fields_found = 0;
    int configured_bits;

    ignored.sa_handler = SIG_IGN;
    caught.sa_handler = unused_handler;
    if (sigemptyset(&ignored.sa_mask) == -1 ||
        sigemptyset(&caught.sa_mask) == -1 ||
        sigaction(SIGUSR1, &ignored, NULL) == -1 ||
        sigaction(SIGUSR2, &caught, NULL) == -1 ||
        sigemptyset(&block_set) == -1 ||
        sigaddset(&block_set, SIGTERM) == -1 ||
        sigprocmask(SIG_BLOCK, &block_set, &previous_mask) == -1) {
        perror("configure signal state");
        return EXIT_FAILURE;
    }

    status_file = fopen("/proc/self/status", "r");
    if (status_file == NULL) {
        perror("fopen /proc/self/status");
        return EXIT_FAILURE;
    }

    errno = 0;
    while (getline(&line, &capacity, status_file) != -1) {
        if (strncmp(line, "SigBlk:", 7U) == 0) {
            blocked_mask = strtoull(line + 7, NULL, 16);
            fields_found += 1;
        } else if (strncmp(line, "SigIgn:", 7U) == 0) {
            ignored_mask = strtoull(line + 7, NULL, 16);
            fields_found += 1;
        } else if (strncmp(line, "SigCgt:", 7U) == 0) {
            caught_mask = strtoull(line + 7, NULL, 16);
            fields_found += 1;
        }
    }
    if (ferror(status_file)) {
        perror("getline /proc/self/status");
        free(line);
        (void)fclose(status_file);
        return EXIT_FAILURE;
    }
    free(line);
    if (fclose(status_file) == EOF) {
        perror("fclose /proc/self/status");
        return EXIT_FAILURE;
    }

    configured_bits = mask_contains(blocked_mask, SIGTERM) &&
                      mask_contains(ignored_mask, SIGUSR1) &&
                      mask_contains(caught_mask, SIGUSR2);
    if (sigprocmask(SIG_SETMASK, &previous_mask, NULL) == -1) {
        perror("restore signal mask");
        return EXIT_FAILURE;
    }

    printf("mask_fields_found=%s configured_bits_visible=%s\n",
           fields_found == 3 ? "yes" : "no",
           configured_bits ? "yes" : "no");
    printf("procfs_is_linux_specific=yes portable_apis_remain_authoritative=yes\n");

    return fields_found == 3 && configured_bits
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
