/*
 * Exercise 03.06 — Read a bounded string up to a chosen delimiter
 *
 * Purpose:
 *   Build a small stream helper for input whose logical fields end at a comma
 *   rather than a newline, without storing the delimiter in the result.
 *
 * Linux behavior:
 *   Repeated fgetc() calls normally consume bytes from the C library's existing
 *   input buffer, not through one read() system call per character. This loop
 *   adds function-call work but retains the syscall amortization of stdio.
 */

#include <stdio.h>
#include <stdlib.h>

#define INPUT_PATH "build/chapters/03-buffered-io/data/delimited_input.txt"

enum { FIELD_CAPACITY = 16 };

/*
 * Return 1 for one field, 0 for clean EOF before another field, and -1 for an
 * input error or a field that cannot fit with its terminating null byte.
 */
static int read_field(FILE *stream,
                      char *destination,
                      size_t capacity,
                      int delimiter)
{
    size_t used = 0;
    int byte;

    if (capacity == 0) {
        return -1;
    }

    while ((byte = fgetc(stream)) != EOF) {
        if (byte == delimiter) {
            destination[used] = '\0';
            return 1;
        }

        if (used + 1 >= capacity) {
            return -1;
        }
        destination[used++] = (char) byte;
    }

    if (ferror(stream)) {
        return -1;
    }

    destination[used] = '\0';
    return used == 0 ? 0 : 1;
}

int main(void)
{
    char field[FIELD_CAPACITY];
    FILE *stream = fopen(INPUT_PATH, "w+");
    size_t field_number = 0;
    int result;

    if (stream == NULL) {
        perror("fopen " INPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fputs("alpha,beta,gamma", stream) == EOF || fflush(stream) == EOF) {
        perror("initialize delimited input");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }
    rewind(stream);

    while ((result = read_field(stream, field, sizeof(field), ',')) == 1) {
        ++field_number;
        printf("field %zu=%s\n", field_number, field);
    }

    if (result == -1) {
        fputs("failed to read a bounded field\n", stderr);
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose " INPUT_PATH);
        return EXIT_FAILURE;
    }

    return field_number == 3 ? EXIT_SUCCESS : EXIT_FAILURE;
}
