/*
 * Experiment - Compete at two nice values on one CPU
 *
 * Prediction:
 *   Under sustained contention, the default-nice worker should usually
 *   complete more loop iterations than the nice-10 worker.
 *
 * Observation rule:
 *   This is evidence, not a benchmark. CFS history, virtualization, frequency
 *   changes, and measurement overhead affect the ratio. No fixed winner is
 *   asserted by the automated tests.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

struct worker_report {
    int nice_value;
    unsigned long long iterations;
};

static long long elapsed_ns(const struct timespec *start,
                            const struct timespec *now)
{
    return ((long long)now->tv_sec - (long long)start->tv_sec) * 1000000000LL +
           ((long long)now->tv_nsec - (long long)start->tv_nsec);
}

static void worker(int start_fd, int report_fd, size_t cpu, int nice_value)
{
    cpu_set_t one_cpu;
    struct timespec start;
    struct timespec now;
    struct worker_report report;
    char token;

    CPU_ZERO(&one_cpu);
    CPU_SET(cpu, &one_cpu);
    if (sched_setaffinity(0, sizeof(one_cpu), &one_cpu) == -1 ||
        setpriority(PRIO_PROCESS, 0, nice_value) == -1 ||
        read(start_fd, &token, 1U) != 1) {
        _exit(2);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        _exit(3);
    }

    report.nice_value = nice_value;
    report.iterations = 0U;
    do {
        ++report.iterations;
        if ((report.iterations & 0x3FFFU) == 0U &&
            clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
            _exit(4);
        }
    } while ((report.iterations & 0x3FFFU) != 0U ||
             elapsed_ns(&start, &now) < 300000000LL);

    if (write(report_fd, &report, sizeof(report)) !=
        (ssize_t)sizeof(report)) {
        _exit(5);
    }
    _exit(0);
}

int main(void)
{
    cpu_set_t allowed;
    size_t selected = (size_t)CPU_SETSIZE;
    int start_pipe[2];
    int report_pipe[2];
    pid_t workers[2];
    struct worker_report reports[2];
    size_t received = 0U;
    int status;

    CPU_ZERO(&allowed);
    if (sched_getaffinity(0, sizeof(allowed), &allowed) == -1) {
        perror("sched_getaffinity");
        return EXIT_FAILURE;
    }
    for (size_t cpu = 0U; cpu < (size_t)CPU_SETSIZE; ++cpu) {
        if (CPU_ISSET(cpu, &allowed)) {
            selected = cpu;
            break;
        }
    }
    if (selected == (size_t)CPU_SETSIZE ||
        pipe(start_pipe) == -1 || pipe(report_pipe) == -1) {
        perror("setup");
        return EXIT_FAILURE;
    }

    for (size_t index = 0U; index < 2U; ++index) {
        workers[index] = fork();
        if (workers[index] == -1) {
            perror("fork");
            return EXIT_FAILURE;
        }
        if (workers[index] == 0) {
            (void)close(start_pipe[1]);
            (void)close(report_pipe[0]);
            worker(start_pipe[0], report_pipe[1], selected,
                   index == 0U ? 0 : 10);
        }
    }

    (void)close(start_pipe[0]);
    (void)close(report_pipe[1]);
    if (write(start_pipe[1], "GG", 2U) != 2) {
        perror("release workers");
        return EXIT_FAILURE;
    }
    (void)close(start_pipe[1]);

    while (received < sizeof(reports)) {
        ssize_t amount = read(report_pipe[0],
                              (unsigned char *)reports + received,
                              sizeof(reports) - received);
        if (amount == -1 && errno == EINTR) {
            continue;
        }
        if (amount <= 0) {
            break;
        }
        received += (size_t)amount;
    }
    (void)close(report_pipe[0]);

    for (size_t index = 0U; index < 2U; ++index) {
        if (waitpid(workers[index], &status, 0) == -1 ||
            !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fputs("worker failed\n", stderr);
            return EXIT_FAILURE;
        }
    }
    if (received != sizeof(reports)) {
        fputs("incomplete reports\n", stderr);
        return EXIT_FAILURE;
    }

    printf("cpu=%zu worker_a_nice=%d iterations=%llu\n", selected,
           reports[0].nice_value, reports[0].iterations);
    printf("cpu=%zu worker_b_nice=%d iterations=%llu\n", selected,
           reports[1].nice_value, reports[1].iterations);
    puts("competition_completed=yes observation_only=yes");
    return EXIT_SUCCESS;
}
