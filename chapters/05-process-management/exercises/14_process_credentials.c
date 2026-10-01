/*
 * Exercise 05.14 — Inspect process credentials safely
 *
 * Purpose:
 *   Read the real and effective user/group IDs plus the number of
 *   supplementary groups without attempting a privileged identity change.
 *
 * Linux behavior:
 *   The real IDs describe the launching identity. Effective IDs participate
 *   in most permission checks. Supplementary groups provide additional group
 *   memberships. Linux credentials also include saved IDs, capabilities, and
 *   a filesystem UID/GID, but changing them safely requires more context.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    uid_t real_uid = getuid();
    uid_t effective_uid = geteuid();
    gid_t real_gid = getgid();
    gid_t effective_gid = getegid();
    int group_count = getgroups(0, NULL);

    if (group_count == -1) {
        perror("getgroups");
        return EXIT_FAILURE;
    }

    printf("uid=%lu euid=%lu gid=%lu egid=%lu supplementary_groups=%d\n",
           (unsigned long)real_uid,
           (unsigned long)effective_uid,
           (unsigned long)real_gid,
           (unsigned long)effective_gid,
           group_count);
    printf("same_user_identity=%s same_group_identity=%s\n",
           real_uid == effective_uid ? "yes" : "no",
           real_gid == effective_gid ? "yes" : "no");
    return EXIT_SUCCESS;
}
