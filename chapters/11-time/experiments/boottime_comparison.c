/*
 * Experiment - Compare CLOCK_BOOTTIME with /proc/uptime
 *
 * Prediction:
 *   Both interfaces approximate elapsed time since boot, including suspended
 *   time for CLOCK_BOOTTIME, and should report nearby values in one sample.
 */

#define _GNU_SOURCE

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    struct timespec boot;
    FILE *stream;
    double proc_uptime;
    double boot_seconds;
    double difference;

    if (clock_gettime(CLOCK_BOOTTIME, &boot) == -1) {
        perror("clock_gettime CLOCK_BOOTTIME");
        return EXIT_FAILURE;
    }
    stream = fopen("/proc/uptime", "r");
    if (stream == NULL) {
        perror("fopen /proc/uptime");
        return EXIT_FAILURE;
    }
    if (fscanf(stream, "%lf", &proc_uptime) != 1) {
        fputs("could not parse /proc/uptime\n", stderr);
        (void)fclose(stream);
        return EXIT_FAILURE;
    }
    if (fclose(stream) == EOF) {
        perror("fclose /proc/uptime");
        return EXIT_FAILURE;
    }

    boot_seconds = (double)boot.tv_sec + (double)boot.tv_nsec / 1000000000.0;
    difference = fabs(boot_seconds - proc_uptime);
    printf("clock_boottime_positive=%s proc_uptime_positive=%s\n",
           boot_seconds > 0.0 ? "yes" : "no",
           proc_uptime > 0.0 ? "yes" : "no");
    printf("samples_within_five_seconds=%s different_interfaces_same_model=yes\n",
           difference < 5.0 ? "yes" : "no");

    return boot_seconds > 0.0 && proc_uptime > 0.0 && difference < 5.0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
