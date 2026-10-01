/*
 * Experiment - Read the Linux I/O priority directly
 *
 * Linux behavior:
 *   glibc does not provide a high-level ioprio_get() wrapper on many systems,
 *   so Linux-specific programs invoke the system call directly. The effective
 *   storage scheduler may ignore this value; an interface existing does not
 *   prove that a device stack enforces it.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef IOPRIO_WHO_PROCESS
#define IOPRIO_WHO_PROCESS 1
#endif
#ifndef IOPRIO_CLASS_SHIFT
#define IOPRIO_CLASS_SHIFT 13
#endif
#define LAB_IOPRIO_CLASS(mask) ((mask) >> IOPRIO_CLASS_SHIFT)
#define LAB_IOPRIO_DATA(mask) ((mask) & ((1 << IOPRIO_CLASS_SHIFT) - 1))

static const char *class_name(int priority_class)
{
    switch (priority_class) {
    case 0:
        return "none/default";
    case 1:
        return "real-time";
    case 2:
        return "best-effort";
    case 3:
        return "idle";
    default:
        return "unknown";
    }
}

int main(void)
{
#ifdef SYS_ioprio_get
    long result;
    int priority_class;
    int priority_data;

    errno = 0;
    result = syscall(SYS_ioprio_get, IOPRIO_WHO_PROCESS, 0);
    if (result == -1) {
        if (errno == ENOSYS) {
            puts("ioprio_supported=no reason=ENOSYS");
            return EXIT_SUCCESS;
        }
        perror("ioprio_get syscall");
        return EXIT_FAILURE;
    }

    priority_class = LAB_IOPRIO_CLASS((int)result);
    priority_data = LAB_IOPRIO_DATA((int)result);
    printf("ioprio_supported=yes raw=%ld class=%d class_name=%s data=%d\n",
           result, priority_class, class_name(priority_class), priority_data);
    puts("device_enforcement_not_assumed=yes");
#else
    puts("ioprio_supported=no reason=SYS_ioprio_get-not-defined");
#endif
    return EXIT_SUCCESS;
}
