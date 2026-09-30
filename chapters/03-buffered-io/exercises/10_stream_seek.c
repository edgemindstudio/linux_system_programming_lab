/*
 * Exercise 03.10 — Seek, query, and rewind a stream
 *
 * Purpose:
 *   Use fseeko(), ftello(), and rewind() while showing how stream operations
 *   consume and update a logical file position.
 *
 * Linux behavior:
 *   The C library coordinates its buffered state with the descriptor offset
 *   when a stream is repositioned. fseeko()/ftello() use off_t and are preferred
 *   over long-based fseek()/ftell() when large-file positions matter.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/stream_seek.txt"

int main(void)
{
    const char initial[] = "0123456789";
    char slice[4] = {0};
    char final[sizeof(initial)] = {0};
    FILE *stream = fopen(OUTPUT_PATH, "w+");
    off_t end_position;
    off_t relative_position;

    if (stream == NULL) {
        perror("fopen " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fwrite(initial, 1, sizeof(initial) - 1, stream) !=
        sizeof(initial) - 1) {
        perror("fwrite initial data");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    end_position = ftello(stream);
    if (end_position == (off_t) -1 || fseeko(stream, -4, SEEK_CUR) == -1) {
        perror("ftello or relative fseeko");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    relative_position = ftello(stream);
    if (relative_position == (off_t) -1) {
        perror("ftello after relative seek");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fread(slice, 1, 3, stream) != 3) {
        if (ferror(stream)) {
            perror("read stream slice");
        } else {
            fputs("unexpected EOF while reading stream slice\n", stderr);
        }
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fseeko(stream, 2, SEEK_SET) == -1 ||
        fwrite("XY", 1, 2, stream) != 2) {
        perror("replace stream bytes");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    /* rewind() returns to the beginning and clears EOF and error indicators. */
    rewind(stream);
    if (fread(final, 1, sizeof(initial) - 1, stream) !=
        sizeof(initial) - 1) {
        if (ferror(stream)) {
            perror("read final stream");
        } else {
            fputs("unexpected EOF while reading final stream\n", stderr);
        }
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("end=%lld after_seek=%lld slice=%s final=%s\n",
           (long long) end_position,
           (long long) relative_position,
           slice,
           final);
    return EXIT_SUCCESS;
}
