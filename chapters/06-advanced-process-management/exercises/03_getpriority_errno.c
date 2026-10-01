/*
 * Exercise 06.03 - Read a nice value without misreading -1
 *
 * Purpose:
 *   Practice the special errno protocol required by getpriority().
 *
 * Linux behavior:
 *   A successful getpriority() call may return -1 because -1 is a valid nice
 *   value. The caller must clear errno before the call and treat -1 as failure
 *   only when errno becomes nonzero.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

int main(void)
{
    int value;
    int saved_errno;

    errno = 0;
    value = getpriority(PRIO_PROCESS, 0);
    saved_errno = errno;

    if (value == -1 && saved_errno != 0) {
        errno = saved_errno;
        perror("getpriority");
        return EXIT_FAILURE;
    }

    printf("nice=%d priority_read=yes errno_after=%d\n", value, saved_errno);
    printf("minus_one_would_be_success=%s errno_protocol_used=yes\n",
           value == -1 ? "yes" : "not-observed");

    return EXIT_SUCCESS;
}
