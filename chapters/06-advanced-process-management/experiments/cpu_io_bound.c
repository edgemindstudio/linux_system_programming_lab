/*
 * Experiment - Compare CPU-bound and blocking work
 *
 * Prediction:
 *   A process that repeatedly sleeps should accumulate voluntary context
 *   switches, while a busy computation should spend more time runnable.
 *
 * Observation rule:
 *   Exact counts depend on the kernel, host load, virtualization, and timer
 *   behavior. Interpret the shape of the result rather than fixed numbers.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

struct report {
    char kind;
    long voluntary;
    long involuntary;
    long user_microseconds;
};

static long long elapsed_ns(const struct timespec *start,
                            const struct timespec *now)
{
    return ((long long)now->tv_sec - (long long)start->tv_sec) * 1000000000LL +
           ((long long)now->tv_nsec - (long long)start->tv_nsec);
}

static int write_report(int fd, char kind)
{
    struct rusage usage;
    struct report result;
    ssize_t written;

    if (getrusage(RUSAGE_SELF, &usage) == -1) {
        return -1;
    }
    result.kind = kind;
    result.voluntary = usage.ru_nvcsw;
    result.involuntary = usage.ru_nivcsw;
    result.user_microseconds = usage.ru_utime.tv_sec * 1000000L +
                               usage.ru_utime.tv_usec;
    written = write(fd, &result, sizeof(result));
    return written == (ssize_t)sizeof(result) ? 0 : -1;
}

static void run_cpu_worker(int output_fd)
{
    struct timespec start;
    struct timespec now;
    volatile unsigned long accumulator = 0UL;

    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        _exit(2);
    }
    do {
        accumulator = accumulator * 33UL + 17UL;
        if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
            _exit(3);
        }
    } while (elapsed_ns(&start, &now) < 150000000LL);

    if (accumulator == 0UL) {
        _exit(4);
    }
    _exit(write_report(output_fd, 'C') == 0 ? 0 : 5);
}

static void run_io_worker(int output_fd)
{
    const struct timespec pause = {0, 7500000L};
    int iteration;

    for (iteration = 0; iteration < 20; ++iteration) {
        struct timespec remaining = pause;

        while (nanosleep(&remaining, &remaining) == -1) {
            if (errno != EINTR) {
                _exit(6);
            }
        }
    }
    _exit(write_report(output_fd, 'I') == 0 ? 0 : 7);
}

int main(void)
{
    int channel[2];
    struct report reports[2];
    int status;
    pid_t cpu_child;
    pid_t io_child;
    size_t received = 0U;

    if (pipe(channel) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    cpu_child = fork();
    if (cpu_child == -1) {
        perror("fork CPU worker");
        return EXIT_FAILURE;
    }
    if (cpu_child == 0) {
        (void)close(channel[0]);
        run_cpu_worker(channel[1]);
    }

    io_child = fork();
    if (io_child == -1) {
        perror("fork I/O worker");
        return EXIT_FAILURE;
    }
    if (io_child == 0) {
        (void)close(channel[0]);
        run_io_worker(channel[1]);
    }

    (void)close(channel[1]);
    while (received < sizeof(reports)) {
        ssize_t amount = read(channel[0],
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
    (void)close(channel[0]);

    if (waitpid(cpu_child, &status, 0) == -1 ||
        !WIFEXITED(status) || WEXITSTATUS(status) != 0 ||
        waitpid(io_child, &status, 0) == -1 ||
        !WIFEXITED(status) || WEXITSTATUS(status) != 0 ||
        received != sizeof(reports)) {
        fputs("worker failure\n", stderr);
        return EXIT_FAILURE;
    }

    for (size_t index = 0U; index < 2U; ++index) {
        printf("kind=%c voluntary=%ld involuntary=%ld user_us=%ld\n",
               reports[index].kind, reports[index].voluntary,
               reports[index].involuntary,
               reports[index].user_microseconds);
    }
    puts("reports_collected=yes observation_is_environment_dependent=yes");
    return EXIT_SUCCESS;
}
