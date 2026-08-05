#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    const pid_t process_id = getpid();

    printf("Linux System Programming Laboratory\n");
    printf("Smoke test completed successfully.\n");
    printf("Process ID: %ld\n", (long)process_id);

    return EXIT_SUCCESS;
}
