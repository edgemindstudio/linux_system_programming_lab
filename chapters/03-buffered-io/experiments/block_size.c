/*
 * Experiment — Preferred I/O block size and the stdio buffer constant
 *
 * Question:
 *   What sizes does this environment expose for efficient file I/O?
 *
 * Prediction:
 *   fstat() will report a positive preferred transfer size in st_blksize, and
 *   the C library's BUFSIZ constant will also be a positive, conventional size.
 *
 * Interpretation:
 *   st_blksize is a performance hint, not the file's allocation-unit size and
 *   not a correctness requirement. BUFSIZ is a C-library constant. The values
 *   may differ because they describe different layers and policies.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define OUTPUT_PATH "build/chapters/03-buffered-io/data/block_size.txt"

int main(void)
{
    struct stat info;
    FILE *stream = fopen(OUTPUT_PATH, "w+");
    int fd;

    if (stream == NULL) {
        perror("fopen " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    fd = fileno(stream);
    if (fd == -1 || fstat(fd, &info) == -1) {
        perror("fileno or fstat");
        (void) fclose(stream);
        return EXIT_FAILURE;
    }

    if (fclose(stream) == EOF) {
        perror("fclose " OUTPUT_PATH);
        return EXIT_FAILURE;
    }

    printf("BUFSIZ=%d preferred_block_size=%lld\n",
           BUFSIZ,
           (long long) info.st_blksize);
    return EXIT_SUCCESS;
}
