#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
cd "$repository_root"

exercise_dir=build/chapters/06-advanced-process-management/exercises
experiment_dir=build/chapters/06-advanced-process-management/experiments
data_dir=build/chapters/06-advanced-process-management/data

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

run_program()
{
    if command -v timeout >/dev/null 2>&1; then
        timeout 15s "$@"
    else
        "$@"
    fi
}

output=$(run_program "$exercise_dir/01_scheduler_snapshot")
assert_contains "$output" 'policy_known=yes allowed_cpu_positive=yes online_cpu_positive=yes'

output=$(run_program "$exercise_dir/02_yield_processor")
assert_contains "$output" 'elapsed_nonnegative=yes yield_is_not_sleep=yes'

output=$(run_program "$exercise_dir/03_getpriority_errno")
assert_contains "$output" 'priority_read=yes'
assert_contains "$output" 'errno_protocol_used=yes'

output=$(run_program "$exercise_dir/04_lower_own_priority")
assert_contains "$output" 'priority_not_raised=yes shell_unchanged=yes'

output=$(run_program "$exercise_dir/05_priority_inheritance")
assert_contains "$output" 'child_inherited=yes values_match=yes'

output=$(run_program "$exercise_dir/06_affinity_inventory")
assert_contains "$output" 'mask_nonempty=yes identifiers_ordered=yes'

output=$(run_program "$exercise_dir/07_affinity_pin_restore")
assert_contains "$output" 'pin_verified=yes restore_verified=yes'

output=$(run_program "$exercise_dir/08_policy_inventory")
assert_contains "$output" 'policy_read=yes parameter_read=yes'

output=$(run_program "$exercise_dir/09_policy_priority_ranges")
assert_contains "$output" 'normal_priority_zero=yes realtime_ranges_valid=yes'

output=$(run_program "$exercise_dir/10_safe_policy_update")
if [[ $output == *'safe_update_skipped=yes'* ]]; then
    assert_contains "$output" 'reason=already-realtime'
else
    assert_contains "$output" 'safe_update_applied=yes policy_unchanged=yes priority_unchanged=yes'
fi

output=$(run_program "$exercise_dir/11_rr_interval")
assert_contains "$output" 'interval_query=yes representation_valid=yes'

output=$(run_program "$exercise_dir/12_realtime_capability_boundary")
assert_contains "$output" 'boundary_inspected=yes realtime_policy_changed=no'

output=$(run_program "$exercise_dir/13_resource_limits_inventory")
assert_contains "$output" 'RLIMIT_NOFILE'
assert_contains "$output" 'RLIMIT_MEMLOCK'
assert_contains "$output" 'limits_read=yes soft_is_active=yes hard_is_ceiling=yes'

output=$(run_program "$exercise_dir/14_lower_soft_nofile")
assert_contains "$output" 'temporary_applied=yes original_restored=yes'

output=$(run_program "$exercise_dir/15_rlimit_fsize_enforcement")
assert_contains "$output" 'file_size=1024 child_exit=0'
assert_contains "$output" 'size_limited=yes parent_survived=yes'

output=$(run_program "$exercise_dir/16_limit_inheritance")
assert_contains "$output" 'limit_inherited=yes child_exit=0 parent_restored=yes'

output=$(run_program "$exercise_dir/17_exec_limit_preservation")
assert_contains "$output" 'limit_preserved_across_exec=yes child_exit=0'

output=$(run_program "$exercise_dir/18_mlock_one_page")
assert_contains "$output" 'page_prefaulted=yes'
if [[ $output == *'mlock_supported=yes'* ]]; then
    assert_contains "$output" 'lock_released=yes'
else
    assert_contains "$output" 'mlock_supported=no'
fi

output=$(run_program "$experiment_dir/cpu_io_bound")
assert_contains "$output" 'reports_collected=yes observation_is_environment_dependent=yes'

output=$(run_program "$experiment_dir/cfs_nice_competition")
assert_contains "$output" 'competition_completed=yes observation_only=yes'

output=$(run_program "$experiment_dir/affinity_migrations")
assert_contains "$output" 'sampling_completed=yes zero_migrations_is_valid=yes'

output=$(run_program "$experiment_dir/io_priority_inventory")
assert_contains "$output" 'ioprio_supported='

output=$(run_program "$experiment_dir/memlock_faults")
assert_contains "$output" 'pages_touched=64'
assert_contains "$output" 'prefault_before_lock=yes'

output=$(run_program "$experiment_dir/nice_permission_probe")
assert_contains "$output" 'probe_isolated_in_child=yes parent_priority_unchanged=yes'

output=$(run_program "$experiment_dir/proc_sched_inventory")
assert_contains "$output" 'proc_sched_read=yes'
assert_contains "$output" 'kernel_specific_interface=yes field_stability_not_assumed=yes'

printf 'PASS: all deterministic Chapter 6 checks completed\n'
