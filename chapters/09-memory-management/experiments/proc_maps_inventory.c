/*
 * Experiment 09.A - Inventory this process's mapped memory regions
 *
 * Purpose:
 *   Read /proc/self/maps and connect the book's address-space model to the
 *   regions Linux exposes for the currently running process.
 *
 * Linux behavior:
 *   Each line describes one virtual memory area. Addresses and ordering vary
 *   because of ASLR, the dynamic loader, libraries, and allocator decisions,
 *   so this experiment checks properties instead of fixed addresses.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    FILE *maps = fopen("/proc/self/maps", "r");
    char *line = NULL;
    size_t capacity = 0U;
    size_t mapping_count = 0U;
    int saw_heap = 0;
    int saw_stack = 0;

    if (maps == NULL) {
        perror("fopen /proc/self/maps");
        return EXIT_FAILURE;
    }

    while (getline(&line, &capacity, maps) != -1) {
        ++mapping_count;
        if (strstr(line, "[heap]") != NULL) {
            saw_heap = 1;
        }
        if (strstr(line, "[stack]") != NULL) {
            saw_stack = 1;
        }
    }

    if (ferror(maps) != 0) {
        perror("getline /proc/self/maps");
        free(line);
        (void)fclose(maps);
        return EXIT_FAILURE;
    }
    free(line);
    if (fclose(maps) == EOF) {
        perror("fclose /proc/self/maps");
        return EXIT_FAILURE;
    }

    printf("mapping_count=%zu heap_mapping=%s stack_mapping=%s\n",
           mapping_count,
           saw_heap != 0 ? "yes" : "no",
           saw_stack != 0 ? "yes" : "no");
    printf("count_positive=%s addresses_not_assumed=yes procfs_snapshot=yes\n",
           mapping_count > 0U ? "yes" : "no");

    return mapping_count > 0U && saw_heap != 0 && saw_stack != 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
