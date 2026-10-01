/*
 * Exercise 05.08 — Registering normal-exit handlers
 *
 * Purpose:
 *   Register three cleanup functions and observe that a normal exit calls
 *   them in the reverse order of registration.
 *
 * Linux behavior:
 *   atexit() records user-space callbacks in the C runtime. Returning from
 *   main() is equivalent to calling exit(), so the handlers run. They would
 *   not run after _exit(), a fatal signal, or a successful exec.
 */

#include <stdio.h>
#include <stdlib.h>

static void first_handler(void)
{
    puts("handler=first");
}

static void second_handler(void)
{
    puts("handler=second");
}

static void third_handler(void)
{
    puts("handler=third");
}

int main(void)
{
    if (atexit(first_handler) != 0 ||
        atexit(second_handler) != 0 ||
        atexit(third_handler) != 0) {
        fprintf(stderr, "atexit registration failed\n");
        return EXIT_FAILURE;
    }

    puts("main=returning");
    return EXIT_SUCCESS;
}
