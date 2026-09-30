/*
 * Exercise 03.08 — Write strings and formatted values
 *
 * Purpose:
 *   Combine fputs() for existing strings and fprintf() for converted values,
 *   then read the resulting text through the same stream abstraction.
 *
 * Linux behavior:
 *   Formatting and string handling occur in the C library before resulting
 *   bytes enter the stream buffer. Those bytes may remain in user space until
 *   the buffer fills, fflush() is called, or fclose() flushes the stream.
 */

#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/formatted_output.txt"

int main(void)
{
    char line[128];
    FILE *stream = fopen(OUTPUT_PATH, "w");

    if (stream == NULL) {
        perror("fopen " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fputs("compiler=", stream) == EOF ||
        fprintf(stream, "%s\n", "clang") < 0 ||
        fprintf(stream, "optimization=%d\n", 0) < 0 ||
        fputs("warnings=strict\n", stream) == EOF) {
        perror("write formatted output");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose formatted output");
        return EXIT_FAILURE;
    }

    stream = fopen(OUTPUT_PATH, "r");
    if (stream == NULL) {
        perror("reopen formatted output");
        return EXIT_FAILURE;
    }

    puts("formatted file:");
    while (fgets(line, sizeof(line), stream) != NULL) {
        if (fputs(line, stdout) == EOF) {
            perror("fputs stdout");
            (void) fclose(stream);
            return EXIT_FAILURE;
        }
    }

    if (ferror(stream)) {
        perror("read formatted output");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose formatted input");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
