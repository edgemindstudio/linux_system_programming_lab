#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 1 ]]; then
    printf 'usage: %s PROGRAM_NAME [PROGRAM_ARGUMENT ...]\n' "$0" >&2
    printf 'example: %s 01_writev_record\n' "$0" >&2
    printf 'example: %s linear_vs_vectored vectored\n' "$0" >&2
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
    "$repository_root/build/chapters/04-advanced-file-io/exercises/$program_name" \
    "$repository_root/build/chapters/04-advanced-file-io/experiments/$program_name"
do
    if [[ -x $candidate ]]; then
        executable=$candidate
        break
    fi
done

if [[ -z $executable ]]; then
    printf 'No built Chapter 4 program named %s was found.\n' "$program_name" >&2
    printf 'Run: make chapter CHAPTER=04\n' >&2
    exit 66
fi

output_directory="$repository_root/chapters/04-advanced-file-io/observations/strace"
output_file="$output_directory/$program_name.strace"
trace_set="openat,close,read,write,pread64,pwrite64,readv,writev"
trace_set+=",epoll_create1,epoll_ctl,epoll_wait"
trace_set+=",mmap,munmap,mremap,mprotect,msync,madvise,fadvise64,readahead"

mkdir -p "$output_directory"

strace \
    -f \
    -e "trace=$trace_set" \
    -o "$output_file" \
    "$executable" "$@"

printf 'Trace written to %s\n' "$output_file"
