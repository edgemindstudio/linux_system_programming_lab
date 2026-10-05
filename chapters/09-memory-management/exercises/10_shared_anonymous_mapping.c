/*
 * Exercise 09.10 - Share an anonymous mapping across fork()
 *
 * Purpose:
 *   Show that MAP_SHARED allows a child process to update bytes that its
 *   parent observes in the same anonymous mapping.
 *
 * Linux behavior:
 *   fork() normally gives parent and child copy-on-write private memory.
 *   A MAP_SHARED mapping is an explicit exception: both processes refer to
 *   shared backing pages, while synchronization still remains the program's
 *   responsibility.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int *shared_value = mmap(NULL,
                             sizeof(*shared_value),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS,
                             -1,
                             0);
    pid_t child;
    int status;
    int child_ok;

    if (shared_value == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    *shared_value = 7;

    child = fork();
    if (child == -1) {
        perror("fork");
        (void)munmap(shared_value, sizeof(*shared_value));
        return EXIT_FAILURE;
    }
    if (child == 0) {
        *shared_value = 42;
        _exit(0);
    }

    if (waitpid(child, &status, 0) == -1) {
        perror("waitpid");
        (void)munmap(shared_value, sizeof(*shared_value));
        return EXIT_FAILURE;
    }
    child_ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;

    printf("child_exit_ok=%s parent_observed=%d shared_update_visible=%s\n",
           child_ok ? "yes" : "no",
           *shared_value,
           child_ok && *shared_value == 42 ? "yes" : "no");
    printf("mapping_shared=yes wait_provided_process_synchronization=yes\n");

    if (munmap(shared_value, sizeof(*shared_value)) == -1) {
        perror("munmap");
        return EXIT_FAILURE;
    }
    return child_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
