#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
exercise_dir="$repository_root/build/chapters/10-signals/exercises"
experiment_dir="$repository_root/build/chapters/10-signals/experiments"

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

output=$(run_program "$exercise_dir/01_signal_identifiers")
assert_contains "$output" 'symbolic_names_used=yes identifiers_distinct=yes'

output=$(run_program "$exercise_dir/02_sigaction_handler")
assert_contains "$output" 'handler_installed=yes signal_handled=yes'
assert_contains "$output" 'handler_used_stdio=no'

output=$(run_program "$exercise_dir/03_pause_until_signal")
assert_contains "$output" 'pause_returned_minus_one=yes errno_is_eintr=yes handled=yes'
assert_contains "$output" 'child_exit_ok=yes'

output=$(run_program "$exercise_dir/04_handler_mask")
assert_contains "$output" 'usr2_pending_during_usr1=yes usr2_handled_after_return=yes'

output=$(run_program "$exercise_dir/05_fork_disposition_inheritance")
assert_contains "$output" 'child_inherited_handler=yes child_exit_ok=yes'
assert_contains "$output" 'parent_flag_unchanged=yes'

output=$(run_program "$exercise_dir/06_exec_dispositions")
assert_contains "$output" 'ignored_disposition_preserved=yes caught_disposition_reset=yes'

output=$(run_program "$exercise_dir/07_signal_descriptions")
assert_contains "$output" 'sigint_description_present=yes sigterm_description_present=yes'
assert_contains "$output" 'description_text_is_locale_sensitive=yes'

output=$(run_program "$exercise_dir/08_null_signal_probe")
assert_contains "$output" 'child_probe_succeeded=yes self_probe_succeeded=yes'
assert_contains "$output" 'signal_delivered=no'

output=$(run_program "$exercise_dir/09_kill_child")
assert_contains "$output" 'child_signaled=yes terminating_signal_is_sigterm=yes'
assert_contains "$output" 'child_reaped=yes unrelated_processes_signaled=no'

output=$(run_program "$exercise_dir/10_raise_self")
assert_contains "$output" 'raise_result_zero=yes handler_completed_before_return=yes'

output=$(run_program "$exercise_dir/11_process_group_signal")
assert_contains "$output" 'isolated_group_created=yes group_signal_sent=yes child_handled=yes'
assert_contains "$output" 'shell_group_untouched=yes'

output=$(run_program "$exercise_dir/12_sig_atomic_flag")
assert_contains "$output" 'stop_requested=yes volatile_sig_atomic_t_used=yes'
assert_contains "$output" 'complex_cleanup_in_handler=no'

output=$(run_program "$exercise_dir/13_signal_set_operations")
assert_contains "$output" 'selected_members_correct=yes deleted_member_absent=yes'

output=$(run_program "$exercise_dir/14_block_and_pending")
assert_contains "$output" 'pending_while_blocked=yes handled_after_unblock=yes'

output=$(run_program "$exercise_dir/15_sigsuspend_wait")
assert_contains "$output" 'signal_handled=yes sigsuspend_returned_minus_one=yes errno_is_eintr=yes'
assert_contains "$output" 'check_and_wait_atomic=yes'

output=$(run_program "$exercise_dir/16_sigwait_sync")
assert_contains "$output" 'sigwait_succeeded=yes received_sigusr1=yes handler_required=no'

output=$(run_program "$exercise_dir/17_siginfo_sender")
assert_contains "$output" 'received_sigusr1=yes sender_pid_matches=yes user_origin=yes'

output=$(run_program "$exercise_dir/18_sigqueue_payload")
assert_contains "$output" 'payload_received=yes value=42 origin_is_sigqueue=yes'

output=$(run_program "$experiment_dir/standard_signal_coalescing")
assert_contains "$output" 'signals_sent=8 pending_bit_set=yes handler_invocations=1'
assert_contains "$output" 'standard_signal_coalesced=yes'

output=$(run_program "$experiment_dir/realtime_signal_queueing")
assert_contains "$output" 'queued=3 received=3 values=10,20,30 sum=60'
assert_contains "$output" 'order_preserved=yes'

output=$(run_program "$experiment_dir/interrupted_read_eintr")
assert_contains "$output" 'first_read_interrupted=yes errno_is_eintr=yes handler_ran=yes'
assert_contains "$output" 'retry_received_byte=yes child_exit_ok=yes sa_restart=no'

output=$(run_program "$experiment_dir/restarted_read")
assert_contains "$output" 'handler_ran=yes read_returned_byte=yes byte=S'
assert_contains "$output" 'sa_restart_used=yes retry_visible_to_application=no'

output=$(run_program "$experiment_dir/self_pipe_bridge")
assert_contains "$output" 'poll_reported_readable=yes byte_received=yes signal_matches=yes'
assert_contains "$output" 'handler_used_only_async_safe_write=yes'

output=$(run_program "$experiment_dir/signalfd_dispatch")
assert_contains "$output" 'record_complete=yes signal_is_sigusr1=yes sender_pid_matches=yes'
assert_contains "$output" 'linux_specific=yes traditional_handler_invoked=no'

output=$(run_program "$experiment_dir/proc_signal_status")
assert_contains "$output" 'mask_fields_found=yes configured_bits_visible=yes'
assert_contains "$output" 'procfs_is_linux_specific=yes'

printf 'PASS: all deterministic Chapter 10 checks completed\n'
