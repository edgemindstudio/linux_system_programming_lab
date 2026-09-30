/*
 * Experiment - Inspect I/O scheduler names exposed by the running kernel
 *
 * Question:
 *   Do the scheduler names on this machine match the historical single-queue
 *   schedulers described in the book?
 *
 * Interpretation:
 *   Modern kernels commonly expose multi-queue choices such as mq-deadline,
 *   kyber, bfq, or none. Virtual devices may expose no selectable scheduler.
 */

#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    DIR *directory = opendir("/sys/block");
    struct dirent *entry;
    size_t inspected = 0;

    if (directory == NULL) {
        printf("devices_inspected=0 sysfs_available=no errno=%d\n", errno);
        return EXIT_SUCCESS;
    }

    while ((entry = readdir(directory)) != NULL) {
        char path[512];
        char schedulers[512];
        FILE *stream;

        if (entry->d_name[0] == '.') {
            continue;
        }

        if (snprintf(path,
                     sizeof(path),
                     "/sys/block/%s/queue/scheduler",
                     entry->d_name) >= (int) sizeof(path)) {
            continue;
        }

        stream = fopen(path, "r");
        if (stream == NULL) {
            continue;
        }

        if (fgets(schedulers, sizeof(schedulers), stream) != NULL) {
            size_t length = strlen(schedulers);

            if (length > 0 && schedulers[length - 1] == '\n') {
                schedulers[length - 1] = '\0';
            }
            printf("device=%s schedulers=%s\n", entry->d_name, schedulers);
            ++inspected;
        }

        if (fclose(stream) == EOF) {
            perror("fclose scheduler sysfs file");
            (void) closedir(directory);
            return EXIT_FAILURE;
        }
    }

    if (closedir(directory) == -1) {
        perror("closedir /sys/block");
        return EXIT_FAILURE;
    }

    printf("devices_inspected=%zu historical_names_may_differ=yes\n", inspected);
    return EXIT_SUCCESS;
}
