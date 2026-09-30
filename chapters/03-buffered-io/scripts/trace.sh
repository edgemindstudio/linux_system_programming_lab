#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 1 ]]; then
    printf 'usage: %s PROGRAM_NAME [PROGRAM_ARGUMENT ...]\n' "$0" >&2
    printf 'example: %s 09_buffered_copy /etc/hosts OUTPUT\n' "$0" >&2
    printf 'example: %s syscall_amortization buffered 4096\n' "$0" >&2
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
    "$repository_root/build/chapters/03-buffered-io/exercises/$program_name" \
    "$repository_root/build/chapters/03-buffered-io/experiments/$program_name"
do
    if [[ -x $candidate ]]; then
        executable=$candidate
        break
    fi
done

if [[ -z $executable ]]; then
    printf 'No built Chapter 3 program named %s was found.\n' "$program_name" >&2
    printf 'Run: make chapter CHAPTER=03\n' >&2
    exit 66
fi

output_directory="$repository_root/chapters/03-buffered-io/observations/strace"
output_file="$output_directory/$program_name.strace"

mkdir -p "$output_directory"

strace \
    -f \
    -e trace=openat,close,read,write,lseek \
    -o "$output_file" \
    "$executable" "$@"

printf 'Trace written to %s\n' "$output_file"
