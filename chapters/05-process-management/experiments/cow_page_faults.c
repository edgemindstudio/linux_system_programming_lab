/*
 * Experiment — Count copy-on-write page faults
 *
 * Prediction:
 *   After fork(), writing one byte in each inherited page causes the child to
 *   take minor faults while Linux creates private writable copies.
 *
 * Measurement caution:
 *   Exact fault counts depend on the allocator, page tables, kernel, and other
 *   runtime work. The stable lesson is that the child incurs additional minor
 *   faults and the parent's buffer remains unchanged.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum { PAGE_COUNT = 128 };

static int exact_io(int fd, void *buffer, size_t length, int writing)
{
    unsigned char *cursor = buffer;

    while (length > 0U) {
        ssize_t count = writing != 0
                            ? write(fd, cursor, length)
                            : read(fd, cursor, length);
        if (count == 0 && writing == 0) {
            return -1;
        }
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        cursor += (size_t)count;
        length -= (size_t)count;
    }
    return 0;
}

int main(void)
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t length;
    unsigned char *memory;
    int report_pipe[2];
    pid_t child;
    long fault_delta;
    int status;

    if (page_size <= 0) {
        fprintf(stderr, "could not determine page size\n");
        return EXIT_FAILURE;
    }
    length = (size_t)page_size * (size_t)PAGE_COUNT;
    memory = calloc(length, 1U);
    if (memory == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }
    for (size_t offset = 0U; offset < length; offset += (size_t)page_size) {
        memory[offset] = 1U;
    }

    if (pipe(report_pipe) == -1) {
        perror("pipe");
        free(memory);
        return EXIT_FAILURE;
    }
    child = fork();
    if (child == -1) {
        perror("fork");
        free(memory);
        return EXIT_FAILURE;
    }
    if (child == 0) {
        struct rusage before;
        struct rusage after;
        long delta;

        (void)close(report_pipe[0]);
        if (getrusage(RUSAGE_SELF, &before) == -1) {
            _exit(120);
        }
        for (size_t offset = 0U; offset < length;
             offset += (size_t)page_size) {
            memory[offset] = 2U;
        }
        if (getrusage(RUSAGE_SELF, &after) == -1) {
            _exit(121);
        }
        delta = after.ru_minflt - before.ru_minflt;
        if (exact_io(report_pipe[1], &delta, sizeof(delta), 1) == -1) {
            _exit(122);
        }
        _exit(0);
    }

    (void)close(report_pipe[1]);
    if (exact_io(report_pipe[0], &fault_delta,
                 sizeof(fault_delta), 0) == -1) {
        perror("read fault count");
        free(memory);
        return EXIT_FAILURE;
    }
    (void)close(report_pipe[0]);
    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        free(memory);
        return EXIT_FAILURE;
    }

    printf("pages_written=%d minor_fault_delta=%ld fault_delta_positive=%s\n",
           PAGE_COUNT,
           fault_delta,
           fault_delta > 0 ? "yes" : "no");
    printf("parent_first_byte=%u parent_unchanged=%s\n",
           (unsigned int)memory[0],
           memory[0] == 1U ? "yes" : "no");
    free(memory);
    return EXIT_SUCCESS;
}
