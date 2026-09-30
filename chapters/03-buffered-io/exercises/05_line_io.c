/*
 * Exercise 03.05 — Read logical lines through a small fgets() buffer
 *
 * Purpose:
 *   Show that fgets() is bounded and safe, but one call does not necessarily
 *   return one complete logical line when the destination array is small.
 *
 * Linux behavior:
 *   fgets() stores at most size - 1 bytes and always terminates a successful
 *   result with '\0'. It retains a newline when that newline fits. A caller
 *   must assemble multiple chunks when a line is longer than the buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_PATH "build/chapters/03-buffered-io/data/line_input.txt"

enum { BUFFER_SIZE = 8 };

int main(void)
{
    const char text[] = "short\nvery-long-line\nlast-no-newline";
    char buffer[BUFFER_SIZE];
    FILE *stream = fopen(INPUT_PATH, "w+");
    size_t chunks = 0;
    size_t lines = 0;
    int last_chunk_ended_line = 1;

    if (stream == NULL) {
        perror("fopen " INPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fputs(text, stream) == EOF || fflush(stream) == EOF) {
        perror("initialize line input");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }
    rewind(stream);

    while (fgets(buffer, sizeof(buffer), stream) != NULL) {
        size_t length = strlen(buffer);

        ++chunks;
        last_chunk_ended_line = length > 0 && buffer[length - 1] == '\n';
        if (last_chunk_ended_line) {
            ++lines;
        }

        printf("chunk %zu: bytes=%zu newline=%s text=\"%s\"\n",
               chunks,
               length,
               last_chunk_ended_line ? "yes" : "no",
               buffer);
    }

    if (ferror(stream)) {
        perror("fgets");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    /* Count a final unterminated line after EOF. */
    if (chunks > 0 && !last_chunk_ended_line) {
        ++lines;
    }

    if (fclose(stream) == EOF) {
        perror("fclose " INPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("summary: chunks=%zu logical_lines=%zu\n", chunks, lines);
    return EXIT_SUCCESS;
}
