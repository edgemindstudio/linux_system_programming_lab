#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 1 ]]; then
    printf 'usage: %s PROGRAM_NAME [PROGRAM_ARGUMENT ...]\n' "$0" >&2
    printf 'example: %s 01_stat_metadata\n' "$0" >&2
    printf 'example: %s inotify_rename_cookie\n' "$0" >&2
    exit 64
fi

if ! command -v strace >/dev/null 2>&1; then
    printf 'strace is required but was not found in PATH\n' >&2
    exit 69
fi

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
program_name=$1
shift
executable=""

for candidate in \
    "$repository_root/build/chapters/08-file-and-directory-management/exercises/$program_name" \
    "$repository_root/build/chapters/08-file-and-directory-management/experiments/$program_name"
do
    if [[ -x $candidate ]]; then
        executable=$candidate
        break
    fi
done

if [[ -z $executable ]]; then
    printf 'No built Chapter 8 program named %s was found.\n' "$program_name" >&2
    printf 'Run: make chapter CHAPTER=08\n' >&2
    exit 66
fi

output_directory="$repository_root/chapters/08-file-and-directory-management/observations/strace"
output_file="$output_directory/$program_name.strace"
trace_set="%file,read,write,close,lseek,getcwd,chdir,fchdir,getdents64"
trace_set+=",ioctl,inotify_init,inotify_init1,inotify_add_watch,inotify_rm_watch"
trace_set+=",exit,exit_group"

mkdir -p "$output_directory"

strace \
    -f \
    -yy \
    -e "trace=$trace_set" \
    -o "$output_file" \
    "$executable" "$@"

printf 'Trace written to %s\n' "$output_file"
