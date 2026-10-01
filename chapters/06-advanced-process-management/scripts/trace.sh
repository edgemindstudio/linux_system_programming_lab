#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 1 ]]; then
    printf 'usage: %s PROGRAM_NAME [PROGRAM_ARGUMENT ...]\n' "$0" >&2
    printf 'example: %s 07_affinity_pin_restore\n' "$0" >&2
    printf 'example: %s nice_permission_probe\n' "$0" >&2
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
    "$repository_root/build/chapters/06-advanced-process-management/exercises/$program_name" \
    "$repository_root/build/chapters/06-advanced-process-management/experiments/$program_name"
do
    if [[ -x $candidate ]]; then
        executable=$candidate
        break
    fi
done

if [[ -z $executable ]]; then
    printf 'No built Chapter 6 program named %s was found.\n' "$program_name" >&2
    printf 'Run: make chapter CHAPTER=06\n' >&2
    exit 66
fi

output_directory="$repository_root/chapters/06-advanced-process-management/observations/strace"
output_file="$output_directory/$program_name.strace"
trace_set="clone,clone3,fork,vfork,execve,execveat,wait4,exit,exit_group"
trace_set+=",sched_yield,sched_getscheduler,sched_setscheduler,sched_getparam"
trace_set+=",sched_rr_get_interval"
trace_set+=",sched_getaffinity,sched_setaffinity,getpriority,setpriority"
trace_set+=",getrlimit,setrlimit,prlimit64,mlock,munlock"
trace_set+=",openat,close,read,write,pipe,pipe2,dup2,fcntl"

mkdir -p "$output_directory"

strace \
    -f \
    -e "trace=$trace_set" \
    -o "$output_file" \
    "$executable" "$@"

printf 'Trace written to %s\n' "$output_file"
