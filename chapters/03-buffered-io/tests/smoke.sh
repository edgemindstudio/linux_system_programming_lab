#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
cd "$repository_root"

exercise_dir=build/chapters/03-buffered-io/exercises
experiment_dir=build/chapters/03-buffered-io/experiments
data_dir=build/chapters/03-buffered-io/data

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

output=$($exercise_dir/01_standard_streams)
assert_contains "$output" 'stdin  -> fd 0 (expected 0)'
assert_contains "$output" 'stdout -> fd 1 (expected 1)'
assert_contains "$output" 'stderr -> fd 2 (expected 2)'

output=$($exercise_dir/02_fopen_modes)
assert_contains "$output" 'contents after w+ then a:'
assert_contains "$output" 'alpha'
assert_contains "$output" 'beta'

output=$($exercise_dir/03_fdopen_bridge)
assert_contains "$output" 'closed_by_fclose=yes'

output=$($exercise_dir/04_character_io)
assert_contains "$output" 'first=A pushed_back=A next=B output=AB'

output=$($exercise_dir/05_line_io)
assert_contains "$output" 'summary: chunks=7 logical_lines=3'

output=$($exercise_dir/06_delimited_input)
assert_contains "$output" 'field 1=alpha'
assert_contains "$output" 'field 2=beta'
assert_contains "$output" 'field 3=gamma'

output=$($exercise_dir/07_binary_io)
assert_contains "$output" 'elements=4 first=0x01020304 last=0xffffffff'

output=$($exercise_dir/08_formatted_output)
assert_contains "$output" 'compiler=clang'
assert_contains "$output" 'warnings=strict'

$exercise_dir/09_buffered_copy \
    chapters/03-buffered-io/exercises/09_buffered_copy.c \
    "$data_dir/buffered_copy.copy" >/dev/null
cmp chapters/03-buffered-io/exercises/09_buffered_copy.c \
    "$data_dir/buffered_copy.copy"

output=$($exercise_dir/10_stream_seek)
assert_contains "$output" 'end=10 after_seek=6 slice=678 final=01XY456789'

output=$($exercise_dir/11_flush_stream)
assert_contains "$output" 'before fflush=0 after fflush=19'

output=$($exercise_dir/12_eof_error_state)
assert_contains "$output" 'EOF state: feof=1 ferror=0'
assert_contains "$output" 'error state: feof=0 ferror=1'

output=$($exercise_dir/13_control_buffering)
assert_contains "$output" 'full: before=0 after_flush=4'
assert_contains "$output" 'line: before_newline=0 after_newline=5'
assert_contains "$output" 'none: immediately=4'

output=$($exercise_dir/14_manual_stream_locking)
assert_contains "$output" 'record=[one logical record]'

output=$($experiment_dir/block_size)
assert_contains "$output" 'BUFSIZ='
assert_contains "$output" 'preferred_block_size='

output=$($experiment_dir/buffer_visibility)
assert_contains "$output" 'before_flush=0 after_flush=7 data=hidden'

output=$($experiment_dir/line_buffering_pipe)
assert_contains "$output" 'before_newline=EAGAIN after_newline=8 data=partial'

$experiment_dir/syscall_amortization raw 128 >/dev/null
$experiment_dir/syscall_amortization buffered 128 >/dev/null
cmp "$data_dir/amortization_raw.bin" "$data_dir/amortization_buffered.bin"

output=$($experiment_dir/mixing_io_layers)
assert_contains "$output" 'after fflush (defined ordering): ABCraw'

output=$($experiment_dir/threaded_records)
assert_contains "$output" 'records=80 malformed=0 locking=preserved'

printf 'PASS: all deterministic Chapter 3 checks completed\n'
