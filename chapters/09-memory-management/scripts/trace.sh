#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 1 ]]; then
    printf 'usage: %s PROGRAM_NAME [PROGRAM_ARGUMENT ...]\n' "$0" >&2
    printf 'example: %s 09_anonymous_mmap\n' "$0" >&2
    printf 'example: %s demand_paging_faults\n' "$0" >&2
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
    "$repository_root/build/chapters/09-memory-management/exercises/$program_name" \
    "$repository_root/build/chapters/09-memory-management/experiments/$program_name"
do
    if [[ -x $candidate ]]; then
        executable=$candidate
        break
    fi
done

if [[ -z $executable ]]; then
    printf 'No built Chapter 9 program named %s was found.\n' "$program_name" >&2
    printf 'Run: make chapter CHAPTER=09\n' >&2
    exit 66
fi

output_directory="$repository_root/chapters/09-memory-management/observations/strace"
output_file="$output_directory/$program_name.strace"
trace_set="%memory,%process,%file,read,write,close,getrusage,mincore"
trace_set+=",mlock,munlock,mlock2,exit,exit_group"

mkdir -p "$output_directory"

strace \
    -f \
    -yy \
    -e "trace=$trace_set" \
    -o "$output_file" \
    "$executable" "$@"

printf 'Trace written to %s\n' "$output_file"
