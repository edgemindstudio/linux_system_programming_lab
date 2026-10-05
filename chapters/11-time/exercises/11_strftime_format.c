/*
 * Exercise 11.11 - Format broken-down time safely
 *
 * Purpose:
 *   Convert a fixed struct tm into a bounded ISO-like text representation.
 *
 * Linux behavior:
 *   strftime() respects the destination size and returns zero when the buffer
 *   is insufficient. Its textual names and some directives are locale-aware.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(void)
{
    const struct tm value = {
        .tm_sec = 56,
        .tm_min = 34,
        .tm_hour = 12,
        .tm_mday = 5,
        .tm_mon = 9,
        .tm_year = 126,
    };
    char formatted[32];
    size_t length = strftime(formatted, sizeof(formatted),
                             "%Y-%m-%dT%H:%M:%S", &value);
    int matches = strcmp(formatted, "2026-10-05T12:34:56") == 0;

    if (length == 0U) {
        fputs("strftime buffer was insufficient\n", stderr);
        return EXIT_FAILURE;
    }

    printf("formatted=%s length=%zu\n", formatted, length);
    printf("bounded_output=yes expected_text=%s locale_sensitive_names_avoided=yes\n",
           matches ? "yes" : "no");

    return matches ? EXIT_SUCCESS : EXIT_FAILURE;
}
