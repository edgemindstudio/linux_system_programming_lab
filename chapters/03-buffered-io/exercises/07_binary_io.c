/*
 * Exercise 03.07 — Read and write fixed-size binary elements
 *
 * Purpose:
 *   Store an array with fwrite(), recover it with fread(), and interpret each
 *   return value as an element count rather than a byte count.
 *
 * Linux behavior:
 *   Standard I/O treats binary data as uninterpreted bytes. This example is a
 *   same-machine round trip: directly storing integers does not define a
 *   portable file format because byte order and integer representation can
 *   differ across systems.
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/binary_values.bin"

int main(void)
{
    const uint32_t expected[] = {
        UINT32_C(0x01020304),
        UINT32_C(0x10203040),
        UINT32_C(0xaabbccdd),
        UINT32_C(0xffffffff)
    };
    uint32_t actual[sizeof(expected) / sizeof(expected[0])] = {0};
    const size_t count = sizeof(expected) / sizeof(expected[0]);
    FILE *stream = fopen(OUTPUT_PATH, "wb");
    size_t index;

    if (stream == NULL) {
        perror("fopen wb " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fwrite(expected, sizeof(expected[0]), count, stream) != count) {
        perror("fwrite binary values");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose binary output");
        return EXIT_FAILURE;
    }

    stream = fopen(OUTPUT_PATH, "rb");
    if (stream == NULL) {
        perror("fopen rb " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    if (fread(actual, sizeof(actual[0]), count, stream) != count) {
        if (ferror(stream)) {
            perror("fread binary values");
        } else {
            fputs("unexpected EOF in binary file\n", stderr);
        }
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    for (index = 0; index < count; ++index) {
        if (actual[index] != expected[index]) {
            fprintf(stderr, "binary mismatch at element %zu\n", index);
            (void) fclose(stream);
            return EXIT_FAILURE;
        }
    }

    if (fclose(stream) == EOF) {
        perror("fclose binary input");
        return EXIT_FAILURE;
    }

    printf("elements=%zu first=0x%08" PRIx32 " last=0x%08" PRIx32 "\n",
           count, actual[0], actual[count - 1]);
    return EXIT_SUCCESS;
}
