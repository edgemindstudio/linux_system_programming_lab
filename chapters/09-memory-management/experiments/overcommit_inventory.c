/*
 * Experiment 09.G - Read the host's overcommit policy safely
 *
 * Purpose:
 *   Connect allocation success to the Linux virtual-memory policy without
 *   changing sysctls or deliberately exhausting memory.
 *
 * Linux behavior:
 *   vm.overcommit_memory selects a policy (0 heuristic, 1 always, 2 strict).
 *   vm.overcommit_ratio contributes to the strict commit limit. Containers
 *   and administrators may expose different values.
 */

#include <stdio.h>
#include <stdlib.h>

static int read_integer(const char *path, long *value)
{
    FILE *stream = fopen(path, "r");
    int scanned;

    if (stream == NULL) {
        return -1;
    }
    scanned = fscanf(stream, "%ld", value);
    if (fclose(stream) == EOF) {
        return -1;
    }
    return scanned == 1 ? 0 : -1;
}

int main(void)
{
    long mode;
    long ratio;

    if (read_integer("/proc/sys/vm/overcommit_memory", &mode) == -1) {
        perror("read overcommit_memory");
        return EXIT_FAILURE;
    }
    if (read_integer("/proc/sys/vm/overcommit_ratio", &ratio) == -1) {
        perror("read overcommit_ratio");
        return EXIT_FAILURE;
    }

    printf("overcommit_mode=%ld overcommit_ratio=%ld\n", mode, ratio);
    printf("mode_valid=%s ratio_nonnegative=%s read_only_probe=yes\n",
           mode >= 0 && mode <= 2 ? "yes" : "no",
           ratio >= 0 ? "yes" : "no");
    printf("allocation_success_is_not_physical_memory_commitment=yes\n");

    return mode >= 0 && mode <= 2 && ratio >= 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
