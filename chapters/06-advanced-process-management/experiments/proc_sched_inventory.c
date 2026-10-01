/*
 * Experiment - Inspect /proc/self/sched
 *
 * Linux behavior:
 *   /proc exposes scheduler accounting that is richer and more kernel-specific
 *   than the portable scheduling APIs. Field names may vary between kernel
 *   versions, so this experiment records selected lines without making them
 *   part of the stable program interface.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    FILE *stream = fopen("/proc/self/sched", "r");
    char line[512];
    int first_line = 1;
    int selected_lines = 0;

    if (stream == NULL) {
        perror("fopen /proc/self/sched");
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), stream) != NULL) {
        if (first_line || strstr(line, "se.vruntime") != NULL ||
            strstr(line, "nr_switches") != NULL ||
            strstr(line, "policy") != NULL ||
            strstr(line, "prio") != NULL) {
            fputs(line, stdout);
            first_line = 0;
            ++selected_lines;
        }
    }
    if (ferror(stream)) {
        perror("fgets /proc/self/sched");
        (void)fclose(stream);
        return EXIT_FAILURE;
    }
    if (fclose(stream) == EOF) {
        perror("fclose");
        return EXIT_FAILURE;
    }

    printf("proc_sched_read=yes selected_lines=%d\n", selected_lines);
    puts("kernel_specific_interface=yes field_stability_not_assumed=yes");
    return EXIT_SUCCESS;
}
