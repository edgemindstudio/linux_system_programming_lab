#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
cd "$repository_root"

exercise_dir=build/chapters/09-memory-management/exercises
experiment_dir=build/chapters/09-memory-management/experiments

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

run_program()
{
    if command -v timeout >/dev/null 2>&1; then
        timeout 15s "$@"
    else
        "$@"
    fi
}

output=$(run_program "$exercise_dir/01_page_size")
assert_contains "$output" 'page_size_positive=yes power_of_two=yes runtime_query_used=yes'

output=$(run_program "$exercise_dir/02_address_space_regions")
assert_contains "$output" 'regions_distinct=yes address_order_not_assumed=yes'

output=$(run_program "$exercise_dir/03_malloc_array")
assert_contains "$output" 'elements=8 bytes=32 sum=36'
assert_contains "$output" 'ownership_released=yes sum_correct=yes'

output=$(run_program "$exercise_dir/04_calloc_zeroed")
assert_contains "$output" 'initially_zero=yes assigned_sum=28'

output=$(run_program "$exercise_dir/05_realloc_preserves_data")
assert_contains "$output" 'prefix_preserved=yes sum=36'
assert_contains "$output" 'temporary_pointer_used=yes'

output=$(run_program "$exercise_dir/06_checked_allocation")
assert_contains "$output" 'overflow_rejected=yes errno_is_enomem=yes safe_allocation=yes'

output=$(run_program "$exercise_dir/07_posix_memalign")
assert_contains "$output" 'alignment=64 size=256 aligned=yes'
assert_contains "$output" 'ordinary_free_used=yes'

output=$(run_program "$exercise_dir/08_program_break")
assert_contains "$output" 'program_break_query=yes address_nonnull=yes'
assert_contains "$output" 'direct_break_change_avoided=yes'

output=$(run_program "$exercise_dir/09_anonymous_mmap")
assert_contains "$output" 'initially_zero=yes endpoints_writable=yes'
assert_contains "$output" 'file_backing=no release_with_munmap=yes'

output=$(run_program "$exercise_dir/10_shared_anonymous_mapping")
assert_contains "$output" 'child_exit_ok=yes parent_observed=42 shared_update_visible=yes'

output=$(run_program "$exercise_dir/11_malloc_usable_size")
assert_contains "$output" 'usable_at_least_requested=yes'
assert_contains "$output" 'only_requested_bytes_used=yes'

output=$(run_program "$exercise_dir/12_malloc_trim")
assert_contains "$output" 'trim_called=yes zero_is_not_failure=yes allocator_decides_release=yes'

output=$(run_program "$exercise_dir/13_alloca_scope")
assert_contains "$output" 'stack_bytes=64 contents_ok=yes'
assert_contains "$output" 'large_sizes_avoided=yes'

output=$(run_program "$exercise_dir/14_variable_length_array")
assert_contains "$output" 'runtime_count=8 sum=36 sum_correct=yes'
assert_contains "$output" 'bounded_input=yes'

output=$(run_program "$exercise_dir/15_memset_memcmp")
assert_contains "$output" 'equal_before=yes different_after=yes compared_bytes=16'

output=$(run_program "$exercise_dir/16_memmove_overlap")
assert_contains "$output" 'result=ababcd expected=ababcd'
assert_contains "$output" 'overlap_handled=yes memmove_required=yes'

output=$(run_program "$exercise_dir/17_memchr_search")
assert_contains "$output" 'target_found=yes index=3 missing_found=no'

output=$(run_program "$exercise_dir/18_mincore_residency")
assert_contains "$output" 'resident_after_touch=yes'
assert_contains "$output" 'initial_state_not_assumed=yes'

output=$(run_program "$exercise_dir/19_mlock_page")
if [[ $output == *'mlock_supported=yes'* ]]; then
    assert_contains "$output" 'page_locked=yes page_unlocked=yes'
else
    assert_contains "$output" 'mlock_supported=no environmental_boundary=yes'
fi
assert_contains "$output" 'secret_data_not_used=yes'

output=$(run_program "$experiment_dir/proc_maps_inventory")
assert_contains "$output" 'heap_mapping=yes stack_mapping=yes'
assert_contains "$output" 'count_positive=yes addresses_not_assumed=yes'

output=$(run_program "$experiment_dir/demand_paging_faults")
assert_contains "$output" 'pages_touched=128'
assert_contains "$output" 'minor_faults_increased=yes major_fault_count_not_assumed=yes'

output=$(run_program "$experiment_dir/malloc_heap_vs_mmap")
assert_contains "$output" 'allocations_succeeded=yes'
assert_contains "$output" 'allocator_strategy_is_not_an_api_contract=yes'

output=$(run_program "$experiment_dir/realloc_movement")
assert_contains "$output" 'data_preserved=yes'
assert_contains "$output" 'movement_is_allowed=yes old_pointer_not_reused=yes'

output=$(run_program "$experiment_dir/madvise_dontneed")
assert_contains "$output" 'zero_after_dontneed=yes'
assert_contains "$output" 'linux_private_anonymous_semantics_observed=yes'

output=$(run_program "$experiment_dir/memfrob_roundtrip")
assert_contains "$output" 'changed_after_once=yes restored_after_twice=yes'
assert_contains "$output" 'obfuscation_only=yes cryptography=no'

output=$(run_program "$experiment_dir/overcommit_inventory")
assert_contains "$output" 'mode_valid=yes ratio_nonnegative=yes read_only_probe=yes'
assert_contains "$output" 'allocation_success_is_not_physical_memory_commitment=yes'

printf 'PASS: all deterministic Chapter 9 checks completed\n'
