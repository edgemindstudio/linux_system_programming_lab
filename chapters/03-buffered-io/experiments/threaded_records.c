/*
 * Experiment — Preserve multi-call records across threads
 *
 * Question:
 *   Individual stdio calls are thread-safe, but how can three calls become one
 *   indivisible logical record?
 *
 * Prediction:
 *   flockfile() around the complete sequence prevents another cooperating
 *   thread from inserting bytes between the record's opening, body, and close.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/threaded_records.txt"

enum { RECORDS_PER_THREAD = 40 };

struct worker_context {
    FILE *stream;
    char tag;
};

static char thread_failure_marker;

static void *write_records(void *argument)
{
    struct worker_context *context = argument;
    int index;

    for (index = 0; index < RECORDS_PER_THREAD; ++index) {
        char body[16];
        int length = snprintf(body, sizeof(body), "%c:%02d", context->tag, index);

        if (length < 0 || (size_t) length >= sizeof(body)) {
            return &thread_failure_marker;
        }

        flockfile(context->stream);
        if (fputs_unlocked("[", context->stream) == EOF ||
            fputs_unlocked(body, context->stream) == EOF ||
            fputs_unlocked("]\n", context->stream) == EOF) {
            funlockfile(context->stream);
            return &thread_failure_marker;
        }
        funlockfile(context->stream);
    }

    return NULL;
}

static int valid_record(const char *line)
{
    size_t length = strlen(line);

    return length == 7 &&
           line[0] == '[' &&
           (line[1] == 'A' || line[1] == 'B') &&
           line[2] == ':' &&
           line[3] >= '0' && line[3] <= '9' &&
           line[4] >= '0' && line[4] <= '9' &&
           line[5] == ']' &&
           line[6] == '\n';
}

int main(void)
{
    struct worker_context first;
    struct worker_context second;
    pthread_t first_thread;
    pthread_t second_thread;
    void *first_result = NULL;
    void *second_result = NULL;
    FILE *stream = fopen(OUTPUT_PATH, "w");
    char line[32];
    size_t records = 0;
    size_t malformed = 0;
    int first_join_status;
    int second_join_status;
    int read_failed;
    int close_failed;
    int read_error_number = 0;

    if (stream == NULL) {
        perror("fopen " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    first.stream = stream;
    first.tag = 'A';
    second.stream = stream;
    second.tag = 'B';

    if (pthread_create(&first_thread, NULL, write_records, &first) != 0) {
        fputs("first pthread_create failed\n", stderr);
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (pthread_create(&second_thread, NULL, write_records, &second) != 0) {
        fputs("second pthread_create failed\n", stderr);
        (void) pthread_join(first_thread, &first_result);
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    /* Always attempt both joins before examining either worker's result. */
    first_join_status = pthread_join(first_thread, &first_result);
    second_join_status = pthread_join(second_thread, &second_result);
    if (first_join_status != 0 || second_join_status != 0 ||
        first_result != NULL || second_result != NULL) {
        fputs("threaded record writer failed\n", stderr);
        if (first_join_status == 0 && second_join_status == 0) {
            (void) fclose(stream);
        }
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose threaded output");
        return EXIT_FAILURE;
    }

    stream = fopen(OUTPUT_PATH, "r");
    if (stream == NULL) {
        perror("reopen threaded output");
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), stream) != NULL) {
        ++records;
        if (!valid_record(line)) {
            ++malformed;
        }
    }

    read_failed = ferror(stream) != 0;
    if (read_failed) {
        read_error_number = errno;
    }
    close_failed = fclose(stream) == EOF;
    if (read_failed || close_failed) {
        if (read_failed) {
            errno = read_error_number;
            perror("read threaded output");
        } else {
            perror("fclose threaded input");
        }
        return EXIT_FAILURE;
    }

    printf("records=%zu malformed=%zu locking=%s\n",
           records,
           malformed,
           malformed == 0 ? "preserved" : "failed");

    return records == 2U * RECORDS_PER_THREAD && malformed == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
