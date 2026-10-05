/*
 * Exercise 10.07 - Map signal identifiers to descriptions
 *
 * Purpose:
 *   Ask the C library for human-readable descriptions of common signals.
 *
 * Linux behavior:
 *   strsignal() returns a locale-sensitive description. Programs may display
 *   it to people, but must not parse it or expect exact English wording.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *interrupt_description = strsignal(SIGINT);
    const char *termination_description = strsignal(SIGTERM);
    int descriptions_present =
        interrupt_description != NULL && interrupt_description[0] != '\0' &&
        termination_description != NULL && termination_description[0] != '\0';

    printf("sigint_description_present=%s sigterm_description_present=%s\n",
           interrupt_description != NULL && interrupt_description[0] != '\0'
               ? "yes"
               : "no",
           termination_description != NULL && termination_description[0] != '\0'
               ? "yes"
               : "no");
    printf("description_text_is_locale_sensitive=yes parsing_avoided=yes\n");

    return descriptions_present ? EXIT_SUCCESS : EXIT_FAILURE;
}
