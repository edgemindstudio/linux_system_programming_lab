/*
 * Experiment — Inherited stdio buffers after fork()
 *
 * Prediction:
 *   Text buffered before fork() exists in both copied FILE objects. If both
 *   processes perform normal stdio cleanup, the same text reaches the file
 *   twice.
 *
 * Lesson:
 *   Flush streams before fork() when duplicate output would be wrong, or call
 *   _exit() in a post-fork child that cannot exec. This is C-library behavior
 *   layered above the shared kernel open-file description.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define OUTPUT_PATH \
    "build/chapters/05-process-management/data/stdio_fork_duplication.txt"

int main(void)
{
    FILE *stream = fopen(OUTPUT_PATH, "w");
    pid_t child;
    int status;
    FILE *reader;
    char line[64];
    int copies = 0;

    if (stream == NULL) {
        perror("fopen output");
        return EXIT_FAILURE;
    }
    if (setvbuf(stream, NULL, _IOFBF, BUFSIZ) != 0 ||
        fputs("copied-buffer\n", stream) == EOF) {
        fprintf(stderr, "could not prepare buffered output\n");
        return EXIT_FAILURE;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        exit(EXIT_SUCCESS);
    }

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }
    if (fclose(stream) == EOF) {
        perror("fclose output");
        return EXIT_FAILURE;
    }

    reader = fopen(OUTPUT_PATH, "r");
    if (reader == NULL) {
        perror("fopen input");
        return EXIT_FAILURE;
    }
    while (fgets(line, sizeof(line), reader) != NULL) {
        ++copies;
    }
    if (ferror(reader) != 0 || fclose(reader) == EOF) {
        perror("read output");
        return EXIT_FAILURE;
    }

    printf("buffered_before_fork=yes copies_in_file=%d duplicated=%s\n",
           copies,
           copies == 2 ? "yes" : "no");
    return WIFEXITED(status) && WEXITSTATUS(status) == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
