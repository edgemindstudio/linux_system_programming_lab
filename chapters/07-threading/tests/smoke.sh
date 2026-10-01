#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
cd "$repository_root"

exercise_dir=build/chapters/07-threading/exercises
experiment_dir=build/chapters/07-threading/experiments

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

output=$(run_program "$exercise_dir/01_thread_identity")
assert_contains "$output" 'process_shared=yes kernel_tasks_distinct=yes pthread_ids_distinct=yes'

output=$(run_program "$exercise_dir/02_create_and_join")
assert_contains "$output" 'thread_created=yes thread_joined=yes result_visible=yes'

output=$(run_program "$exercise_dir/03_stable_arguments")
assert_contains "$output" 'squares=4,9,16'
assert_contains "$output" 'arguments_stable=yes all_threads_joined=yes'

output=$(run_program "$exercise_dir/04_shared_address_space")
assert_contains "$output" 'address_space_shared=yes join_synchronized=yes'

output=$(run_program "$exercise_dir/05_independent_stacks")
assert_contains "$output" 'addresses_nonzero=yes stack_addresses_distinct=yes'

output=$(run_program "$exercise_dir/06_return_value_ownership")
assert_contains "$output" 'worker_result=42 ownership_transferred=yes'
assert_contains "$output" 'joining_thread_frees_result=yes value_correct=yes'

output=$(run_program "$exercise_dir/07_join_many")
assert_contains "$output" 'threads_created=4 threads_joined=4 total=30'
assert_contains "$output" 'all_resources_reclaimed=yes results_complete=yes'

output=$(run_program "$exercise_dir/08_detached_completion")
assert_contains "$output" 'detached=yes completion_observed=yes value=42'
assert_contains "$output" 'join_attempted=no separate_protocol_used=yes'

output=$(run_program "$exercise_dir/09_main_calls_pthread_exit")
assert_contains "$output" 'initial_thread_action=pthread_exit'
assert_contains "$output" 'worker_completed_after_initial_thread_exit=yes'

output=$(run_program "$exercise_dir/10_compare_thread_ids")
assert_contains "$output" 'created_matches_worker_self=yes'
assert_contains "$output" 'main_differs_from_worker=yes opaque_ids_compared_portably=yes'

output=$(run_program "$exercise_dir/11_deferred_cancellation")
assert_contains "$output" 'cancel_request_accepted=yes joined=yes'
assert_contains "$output" 'deferred_cancellation_observed=yes'

output=$(run_program "$exercise_dir/12_cancellation_cleanup")
assert_contains "$output" 'thread_canceled=yes cleanup_called=yes'
assert_contains "$output" 'worker_allocation_released=yes'

output=$(run_program "$exercise_dir/13_mutex_counter")
assert_contains "$output" 'expected=40000 actual=40000'
assert_contains "$output" 'critical_region_protected=yes all_threads_joined=yes'

output=$(run_program "$exercise_dir/14_trylock_busy")
assert_contains "$output" 'busy_observed=yes worker_blocked=no'

output=$(run_program "$exercise_dir/15_account_withdrawal")
assert_contains "$output" 'one_succeeded=yes no_overdraft=yes invariant_preserved=yes'

output=$(run_program "$exercise_dir/16_lock_order")
assert_contains "$output" 'both_transfers_completed=yes lock_order_consistent=yes total_preserved=yes'

output=$(run_program "$exercise_dir/17_errorcheck_mutex")
assert_contains "$output" 'self_deadlock_detected=yes program_hung=no'

output=$(run_program "$exercise_dir/18_condition_handoff")
assert_contains "$output" 'predicate_loop_used=yes handoff_completed=yes'

output=$(run_program "$experiment_dir/logical_race_atomic")
assert_contains "$output" 'logical_race_observed=yes atomic_transaction_correct=yes'

output=$(run_program "$experiment_dir/mutex_contention")
assert_contains "$output" 'counter_correct=yes contention_is_environment_dependent=yes'

output=$(run_program "$experiment_dir/thread_stack_addresses")
assert_contains "$output" 'local_addresses_distinct=yes'
assert_contains "$output" 'shared_heap_address_identical=yes shared_value=42'

output=$(run_program "$experiment_dir/proc_task_inventory")
assert_contains "$output" 'initial_plus_workers_visible=yes linux_specific_interface=yes'

output=$(run_program "$experiment_dir/thread_per_task")
assert_contains "$output" 'thread_per_task_completed=yes scaling_cost=one-thread-per-live-task'

output=$(run_program "$experiment_dir/bounded_worker_pool")
assert_contains "$output" 'workers_reused=yes bounded_thread_count=yes'

output=$(run_program "$experiment_dir/deadlock_timeout")
assert_contains "$output" 'wait_bounded=yes process_hung=no ordering_still_required=yes'

printf 'PASS: all deterministic Chapter 7 checks completed\n'
