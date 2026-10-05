/*
 * Exercise 09.01 - Discover the system page size
 *
 * Purpose:
 *   Ask Linux for the page size used by this process and connect that unit to
 *   virtual-memory alignment and page-granular kernel operations.
 *
 * Linux behavior:
 *   A process addresses bytes, but the kernel and memory-management unit map
 *   memory in pages. sysconf() reports the runtime value rather than forcing
 *   the program to assume a common size such as 4096 bytes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    long page_size = sysconf(_SC_PAGESIZE);
    int power_of_two;

    if (page_size <= 0) {
        perror("sysconf _SC_PAGESIZE");
        return EXIT_FAILURE;
    }

    power_of_two = (page_size & (page_size - 1L)) == 0L;
    printf("page_size=%ld pointer_size=%zu\n", page_size, sizeof(void *));
    printf("page_size_positive=yes power_of_two=%s runtime_query_used=yes\n",
           power_of_two ? "yes" : "no");

    return EXIT_SUCCESS;
}
