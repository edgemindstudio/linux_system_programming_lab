#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
cd "$repository_root"

exercise_dir=build/exercises
experiment_dir=build/experiments
data_dir=build/chapter02/data

mkdir -p "$data_dir"

assert_contains()
{
    local text=$1
    local expected=$2

    if [[ $text != *"$expected"* ]]; then
        printf 'FAIL: expected output to contain: %s\n' "$expected" >&2
        printf 'actual output:\n%s\n' "$text" >&2
        exit 1
    fi
}

output=$($exercise_dir/chapter02_01_standard_fds)
assert_contains "$output" 'STDIN_FILENO = 0'
assert_contains "$output" 'STDOUT_FILENO = 1'
assert_contains "$output" 'STDERR_FILENO = 2'

$exercise_dir/chapter02_02_open_file >/dev/null
$exercise_dir/chapter02_03_access_modes >/dev/null 2>&1
$exercise_dir/chapter02_04_create_file >/dev/null
$exercise_dir/chapter02_05_creat_vs_open >/dev/null

$exercise_dir/chapter02_06_read_file /etc/hosts >"$data_dir/hosts.copy"
cmp /etc/hosts "$data_dir/hosts.copy"

$exercise_dir/chapter02_07_write_file >/dev/null
assert_contains "$(<"$data_dir/write_demo.txt")" 'user-space buffer'

$exercise_dir/chapter02_08_robust_copy \
    exercises/chapter02_08_robust_copy.c \
    "$data_dir/robust_copy.copy" >/dev/null
cmp exercises/chapter02_08_robust_copy.c "$data_dir/robust_copy.copy"

output=$($exercise_dir/chapter02_09_seek_file)
assert_contains "$output" 'offset=6'
assert_contains "$output" '01XY456789'

output=$($exercise_dir/chapter02_10_positional_io)
assert_contains "$output" 'before=0 after pread=0 after pwrite=0'
assert_contains "$output" 'AxyzEFGHIJ'

output=$($exercise_dir/chapter02_11_truncate_file)
assert_contains "$output" 'size=4, current offset=8'

output=$(printf 'ready\n' | $exercise_dir/chapter02_12_select_stdin)
assert_contains "$output" 'stdin ready: read 6 bytes: ready'

$experiment_dir/chapter02_fd_reuse >/dev/null
$experiment_dir/chapter02_open_flags >/dev/null

output=$($experiment_dir/chapter02_permissions_umask)
assert_contains "$output" 'actual mode:    0644'

output=$($experiment_dir/chapter02_nonblocking_pipe)
assert_contains "$output" 'Resource temporarily unavailable'
assert_contains "$output" 'read returned 5 bytes: hello'

$experiment_dir/chapter02_durable_write >/dev/null

if $experiment_dir/chapter02_direct_io >/dev/null 2>&1; then
    printf 'PASS: O_DIRECT experiment supported\n'
else
    direct_status=$?
    if [[ $direct_status -eq 2 ]]; then
        printf 'SKIP: O_DIRECT unsupported by this filesystem\n'
    else
        printf 'FAIL: O_DIRECT experiment exited with %d\n' "$direct_status" >&2
        exit "$direct_status"
    fi
fi

output=$($experiment_dir/chapter02_sparse_file)
assert_contains "$output" 'logical size: 1048579 bytes'
assert_contains "$output" '00 00 00 00 00 00 00 00'

printf 'PASS: all deterministic Chapter 2 checks completed\n'
