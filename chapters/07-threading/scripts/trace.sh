#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 1 ]]; then
    printf 'usage: %s PROGRAM_NAME [PROGRAM_ARGUMENT ...]\n' "$0" >&2
    printf 'example: %s 01_thread_identity\n' "$0" >&2
    printf 'example: %s mutex_contention\n' "$0" >&2
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
    "$repository_root/build/chapters/07-threading/exercises/$program_name" \
    "$repository_root/build/chapters/07-threading/experiments/$program_name"
do
    if [[ -x $candidate ]]; then
        executable=$candidate
        break
    fi
done

if [[ -z $executable ]]; then
    printf 'No built Chapter 7 program named %s was found.\n' "$program_name" >&2
    printf 'Run: make chapter CHAPTER=07\n' >&2
    exit 66
fi

output_directory="$repository_root/chapters/07-threading/observations/strace"
output_file="$output_directory/$program_name.strace"
trace_set="clone,clone3,futex,set_robust_list,rseq,getpid,gettid"
trace_set+=",mmap,mprotect,munmap,openat,close,getdents64"
trace_set+=",clock_gettime,clock_nanosleep,nanosleep,write,exit,exit_group"

mkdir -p "$output_directory"

strace \
    -f \
    -e "trace=$trace_set" \
    -o "$output_file" \
    "$executable" "$@"

printf 'Trace written to %s\n' "$output_file"
