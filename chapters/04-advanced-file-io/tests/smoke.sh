#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
cd "$repository_root"

exercise_dir=build/chapters/04-advanced-file-io/exercises
experiment_dir=build/chapters/04-advanced-file-io/experiments
data_dir=build/chapters/04-advanced-file-io/data

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

output=$($exercise_dir/01_writev_record)
assert_contains "$output" 'segments=3 bytes=30'
assert_contains "$output" 'type=event value=42 status=ok'

output=$($exercise_dir/02_readv_segments)
assert_contains "$output" 'bytes=19 header=HDR1 payload=payload-data trailer=END'

output=$($exercise_dir/03_epoll_readiness)
assert_contains "$output" 'events=1 readable=yes message=ready lifecycle=add-mod-del'

output=$($exercise_dir/04_epoll_timeout)
assert_contains "$output" 'timeout_ms=25 ready_events=0 outcome=timeout'

output=$($exercise_dir/05_epoll_level_triggered)
assert_contains "$output" 'first_wait=1 second_wait=1 first=A remaining=BC'

output=$($exercise_dir/06_epoll_edge_triggered)
assert_contains "$output" 'first_wait=1 second_wait=0 first=A drained=2 stopped=EAGAIN'

output=$($exercise_dir/07_mmap_read)
assert_contains "$output" 'descriptor_closed=yes mapped_text=mapped bytes remain available'

output=$($exercise_dir/08_mmap_shared_write)
assert_contains "$output" 'mapping=shared!! file=shared!! msync=yes'

output=$($exercise_dir/09_mmap_private_copy)
assert_contains "$output" 'memory=private! file=original copy_on_write=yes'

output=$($exercise_dir/10_page_size_alignment)
assert_contains "$output" 'offset='
assert_contains "$output" 'aligned=yes marker=PAGE2'

output=$($exercise_dir/11_mremap_resize)
assert_contains "$output" 'preserved=yes expanded=yes'

output=$($exercise_dir/12_mprotect_region)
assert_contains "$output" 'protection=read-only page_granularity=yes'

output=$($exercise_dir/13_madvise_mapping)
assert_contains "$output" 'advice=sequential+willneed'
assert_contains "$output" 'accepted=yes'

output=$($exercise_dir/14_posix_fadvise)
assert_contains "$output" 'advice=sequential+willneed range=whole-file accepted=yes'

output=$($exercise_dir/15_asynchronous_read)
assert_contains "$output" 'submitted=yes completed=yes bytes=17 text=asynchronous-data'

output=$($exercise_dir/16_inode_order)
assert_contains "$output" 'files=3 ordered=yes strategy=inode-heuristic'

$experiment_dir/linear_vs_vectored linear >/dev/null
$experiment_dir/linear_vs_vectored vectored >/dev/null
cmp "$data_dir/linear_output.txt" "$data_dir/vectored_output.txt"

output=$($experiment_dir/epoll_many_fds)
assert_contains "$output" 'registered=64 ready=3 returned_only_ready=yes'

output=$($experiment_dir/mmap_coherence)
assert_contains "$output" 'descriptor_before_msync=warm coherent=yes synchronization=done'

output=$($experiment_dir/page_faults)
assert_contains "$output" 'pages_touched='
assert_contains "$output" 'minor_fault_delta='
assert_contains "$output" 'major_fault_delta='

output=$($experiment_dir/readahead_probe)
assert_contains "$output" 'readahead_result='

output=$($experiment_dir/aio_parallel_reads)
assert_contains "$output" 'requests=3 completed=3 data=alpha,bravo,gamma'

output=$($experiment_dir/scheduler_inventory)
assert_contains "$output" 'devices_inspected='
assert_contains "$output" 'historical_names_may_differ=yes'

printf 'PASS: all deterministic Chapter 4 checks completed\n'
