#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 1 ]]; then
    printf 'usage: %s PROGRAM_NAME [PROGRAM_ARGUMENT ...]\n' "$0" >&2
    printf 'example: %s 06_read_file /etc/hosts\n' "$0" >&2
    exit 64
fi

repository_root=$(git rev-parse --show-toplevel)
program_name=$1
shift
executable=""

for candidate in \
    "$repository_root/build/chapters/02-file-io/exercises/$program_name" \
    "$repository_root/build/chapters/02-file-io/experiments/$program_name"
do
    if [[ -x $candidate ]]; then
        executable=$candidate
        break
    fi
done

if [[ -z $executable ]]; then
    printf 'No built Chapter 2 program named %s was found.\n' "$program_name" >&2
    printf 'Run: make chapter CHAPTER=02\n' >&2
    exit 66
fi

output_directory="$repository_root/chapters/02-file-io/observations/strace"
output_file="$output_directory/$program_name.strace"

mkdir -p "$output_directory"

strace \
    -o "$output_file" \
    -e trace=openat,read,write,close,lseek,pread64,pwrite64,ftruncate,fsync,fdatasync,select,pselect6 \
    "$executable" "$@"

printf 'Saved trace: %s\n' "$output_file"
