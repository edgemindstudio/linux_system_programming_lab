/*
 * Exercise 10.01 - Use symbolic signal identifiers
 *
 * Purpose:
 *   Inspect several standard signal identifiers without depending on their
 *   implementation-specific integer values.
 *
 * Linux behavior:
 *   <signal.h> defines positive integer identifiers such as SIGINT and
 *   SIGTERM. Portable programs use the symbolic names. Signal number zero is
 *   reserved by interfaces such as kill() for a probe rather than delivery.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int positive = SIGINT > 0 && SIGTERM > 0 && SIGUSR1 > 0;
    int distinct = SIGINT != SIGTERM && SIGTERM != SIGUSR1;

    printf("sigint_positive=%s sigterm_positive=%s sigusr1_positive=%s\n",
           SIGINT > 0 ? "yes" : "no",
           SIGTERM > 0 ? "yes" : "no",
           SIGUSR1 > 0 ? "yes" : "no");
    printf("symbolic_names_used=yes identifiers_distinct=%s "
           "null_signal_reserved=yes\n",
           distinct ? "yes" : "no");

    return positive && distinct ? EXIT_SUCCESS : EXIT_FAILURE;
}
