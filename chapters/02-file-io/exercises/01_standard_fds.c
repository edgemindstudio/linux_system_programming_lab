/*
 * Exercise 02.01 — Standard file descriptors
 *
 * Purpose:
 *   Display the symbolic descriptor numbers that POSIX assigns to a process's
 *   standard input, standard output, and standard error streams.
 *
 * Linux behavior:
 *   A process normally inherits these three open descriptors from its parent
 *   (usually the shell). Shell redirection changes what a descriptor refers to;
 *   it does not change the conventional descriptor number.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    /* Use the symbolic constants instead of embedding the numbers 0, 1, and 2. */
    printf("STDIN_FILENO = %d\n", STDIN_FILENO);
    printf("STDOUT_FILENO = %d\n", STDOUT_FILENO);
    printf("STDERR_FILENO = %d\n", STDERR_FILENO);

    return EXIT_SUCCESS;
}
