#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 1 ]]; then
    printf 'usage: %s PROGRAM_NAME [PROGRAM_ARGUMENT ...]\n' "$0" >&2
    printf 'example: %s 04_realtime_and_monotonic\n' "$0" >&2
    printf 'example: %s timerfd_expirations\n' "$0" >&2
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
    "$repository_root/build/chapters/11-time/exercises/$program_name" \
    "$repository_root/build/chapters/11-time/experiments/$program_name"
do
    if [[ -x $candidate ]]; then
        executable=$candidate
        break
    fi
done

if [[ -z $executable ]]; then
    printf 'No built Chapter 11 program named %s was found.\n' "$program_name" >&2
    printf 'Run: make chapter CHAPTER=11\n' >&2
    exit 66
fi

output_directory="$repository_root/chapters/11-time/observations/strace"
output_file="$output_directory/$program_name.strace"
trace_set='%signal,%process,%file,read,write,close,clock_gettime,clock_getres,gettimeofday,time,times,nanosleep,clock_nanosleep,alarm,setitimer,getitimer,timer_create,timer_settime,timer_gettime,timer_getoverrun,timer_delete,timerfd_create,timerfd_settime,timerfd_gettime'

mkdir -p "$output_directory"

strace \
    -f \
    -yy \
    -e "trace=$trace_set" \
    -o "$output_file" \
    "$executable" "$@"

printf 'Trace written to %s\n' "$output_file"
