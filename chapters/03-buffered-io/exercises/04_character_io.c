/*
 * Exercise 03.04 — Character I/O and one-byte pushback
 *
 * Purpose:
 *   Read bytes with fgetc(), return one byte to the stream with ungetc(), and
 *   write selected bytes with fputc().
 *
 * Linux behavior:
 *   Character-oriented functions usually consume bytes already present in the
 *   stream's user-space buffer. fgetc() returns int so every unsigned-char value
 *   remains distinguishable from the negative EOF sentinel. ungetc() provides
 *   at least one byte of pushback and clears the stream's EOF indicator.
 */

#include <stdio.h>
#include <stdlib.h>

#define INPUT_PATH "build/chapters/03-buffered-io/data/character_input.txt"
#define OUTPUT_PATH "build/chapters/03-buffered-io/data/character_output.txt"

int main(void)
{
    FILE *input = fopen(INPUT_PATH, "w+");
    FILE *output;
    int first;
    int pushed_back;
    int next;
    int close_failed = 0;

    if (input == NULL) {
        perror("fopen " INPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fputs("ABC", input) == EOF || fflush(input) == EOF) {
        perror("initialize character input");
        (void) fclose(input);
        return EXIT_FAILURE;
    }
    rewind(input);

    first = fgetc(input);
    if (first == EOF || ungetc(first, input) == EOF) {
        perror("fgetc or ungetc");
        (void) fclose(input);
        return EXIT_FAILURE;
    }

    pushed_back = fgetc(input);
    next = fgetc(input);
    if (pushed_back == EOF || next == EOF) {
        perror("read pushed-back characters");
        (void) fclose(input);
        return EXIT_FAILURE;
    }

    output = fopen(OUTPUT_PATH, "w");
    if (output == NULL) {
        perror("fopen " OUTPUT_PATH);
        (void) fclose(input);
        return EXIT_FAILURE;
    }

    if (fputc(pushed_back, output) == EOF || fputc(next, output) == EOF) {
        perror("fputc");
        (void) fclose(input);
        (void) fclose(output);
        return EXIT_FAILURE;
    }

    /* Attempt both closes even if the first close reports an error. */
    if (fclose(input) == EOF) {
        perror("fclose character input");
        close_failed = 1;
    }
    if (fclose(output) == EOF) {
        perror("fclose character output");
        close_failed = 1;
    }
    if (close_failed) {
        return EXIT_FAILURE;
    }

    printf("first=%c pushed_back=%c next=%c output=%c%c\n",
           first, pushed_back, next, pushed_back, next);
    return EXIT_SUCCESS;
}
