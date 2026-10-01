/*
 * Experiment — Read PID limits and PID-namespace context at runtime
 *
 * Purpose:
 *   Replace historical constants and host-only assumptions with evidence from
 *   the current Linux system.
 *
 * Linux behavior:
 *   /proc/sys/kernel/pid_max exposes the current allocation ceiling. PID 1's
 *   command name depends on the environment, and NSpid may list one PID per
 *   nested PID namespace. These values can differ between a host, WSL, and a
 *   container.
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int read_long_file(const char *path, long *value)
{
    FILE *stream = fopen(path, "r");
    int matched;

    if (stream == NULL) {
        return -1;
    }
    matched = fscanf(stream, "%ld", value);
    (void)fclose(stream);
    return matched == 1 ? 0 : -1;
}

static int read_first_line(const char *path, char *buffer, size_t length)
{
    FILE *stream = fopen(path, "r");

    if (stream == NULL) {
        return -1;
    }
    if (fgets(buffer, (int)length, stream) == NULL) {
        (void)fclose(stream);
        return -1;
    }
    (void)fclose(stream);
    buffer[strcspn(buffer, "\n")] = '\0';
    return 0;
}

static int namespace_pid_levels(void)
{
    FILE *stream = fopen("/proc/self/status", "r");
    char line[512];
    int levels = 0;

    if (stream == NULL) {
        return -1;
    }
    while (fgets(line, sizeof(line), stream) != NULL) {
        if (strncmp(line, "NSpid:", 6U) == 0) {
            const char *cursor = line + 6;
            while (*cursor != '\0') {
                char *end;
                (void)strtol(cursor, &end, 10);
                if (end != cursor) {
                    ++levels;
                    cursor = end;
                } else {
                    ++cursor;
                }
            }
            break;
        }
    }
    (void)fclose(stream);
    return levels;
}

int main(void)
{
    long pid_max;
    char pid1_name[128];
    int levels;

    if (read_long_file("/proc/sys/kernel/pid_max", &pid_max) == -1 ||
        read_first_line("/proc/1/comm", pid1_name, sizeof(pid1_name)) == -1) {
        perror("read procfs process context");
        return EXIT_FAILURE;
    }
    levels = namespace_pid_levels();
    if (levels == -1) {
        perror("read /proc/self/status");
        return EXIT_FAILURE;
    }

    printf("self_pid=%ld pid_max=%ld pid1_comm=%s namespace_pid_levels=%d\n",
           (long)getpid(),
           pid_max,
           pid1_name,
           levels);
    printf("runtime_values_used=yes\n");
    return EXIT_SUCCESS;
}
