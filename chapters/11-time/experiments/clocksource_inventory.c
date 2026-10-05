/*
 * Experiment - Inspect the active Linux clock source
 *
 * Prediction:
 *   When sysfs exposes clocksource metadata, the current source appears in
 *   the list of sources available to the kernel.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_line(const char *path, char *buffer, size_t capacity)
{
    FILE *stream = fopen(path, "r");
    size_t length;

    if (stream == NULL) {
        return -1;
    }
    if (fgets(buffer, (int)capacity, stream) == NULL) {
        (void)fclose(stream);
        return -1;
    }
    if (fclose(stream) == EOF) {
        return -1;
    }
    length = strlen(buffer);
    if (length > 0U && buffer[length - 1U] == '\n') {
        buffer[length - 1U] = '\0';
    }
    return 0;
}

int main(void)
{
    const char *base = "/sys/devices/system/clocksource/clocksource0/";
    char current[128];
    char available[512];
    char current_path[256];
    char available_path[256];
    int current_length;
    int available_length;

    current_length = snprintf(current_path, sizeof(current_path),
                              "%scurrent_clocksource", base);
    available_length = snprintf(available_path, sizeof(available_path),
                                "%savailable_clocksource", base);
    if (current_length < 0 || (size_t)current_length >= sizeof(current_path) ||
        available_length < 0 ||
        (size_t)available_length >= sizeof(available_path)) {
        fputs("clocksource path construction failed\n", stderr);
        return EXIT_FAILURE;
    }

    if (read_line(current_path, current, sizeof(current)) == -1 ||
        read_line(available_path, available, sizeof(available)) == -1) {
        printf("clocksource_sysfs_available=no environment_dependent=yes\n");
        return EXIT_SUCCESS;
    }

    printf("clocksource_sysfs_available=yes current_nonempty=%s\n",
           current[0] != '\0' ? "yes" : "no");
    printf("current_listed_as_available=%s kernel_selects_clocksource=yes\n",
           strstr(available, current) != NULL ? "yes" : "no");

    return current[0] != '\0' && strstr(available, current) != NULL
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
