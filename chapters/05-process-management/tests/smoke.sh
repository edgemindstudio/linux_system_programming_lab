#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
cd "$repository_root"

exercise_dir=build/chapters/05-process-management/exercises
experiment_dir=build/chapters/05-process-management/experiments
data_dir=build/chapters/05-process-management/data

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
        timeout 10s "$@"
    else
        "$@"
    fi
}

output=$(run_program "$exercise_dir/01_process_identity")
assert_contains "$output" 'pid_positive=yes parent_nonnegative=yes'

output=$(run_program "$exercise_dir/02_fork_return_values")
assert_contains "$output" 'child_pid_matches=yes child_parent_matches=yes child_exit=0'

output=$(run_program "$exercise_dir/03_copy_on_write")
assert_contains "$output" 'parent_value=41 child_before=41 child_after=99'
assert_contains "$output" 'same_virtual_address=yes independent_values=yes'

output=$(run_program "$exercise_dir/04_fork_exec_wait")
assert_contains "$output" 'output=hello-from-exec exited=yes status=0'

output=$(run_program "$exercise_dir/05_exec_arguments_environment")
assert_contains "$output" 'argc=3 arg1=alpha arg2=beta env=execve same_pid=yes'

output=$(run_program "$exercise_dir/06_close_on_exec")
assert_contains "$output" 'ordinary_inherited=yes cloexec_closed=yes'

output=$(run_program "$exercise_dir/07_exit_and__exit")
assert_contains "$output" 'exit_bytes=13 _exit_bytes=0'
assert_contains "$output" 'exit_flushed=yes _exit_skipped_stdio=yes'

output=$(run_program "$exercise_dir/08_atexit_order")
expected_order=$'main=returning\nhandler=third\nhandler=second\nhandler=first'
if [[ $output != "$expected_order" ]]; then
    printf 'FAIL: atexit handlers did not run in reverse order\n' >&2
    printf 'actual output:\n%s\n' "$output" >&2
    exit 1
fi

output=$(run_program "$exercise_dir/09_wait_exit_status")
assert_contains "$output" 'exited=yes exit_status=42'

output=$(run_program "$exercise_dir/10_wait_signal_status")
assert_contains "$output" 'signaled=yes signal_matches_SIGTERM=yes normal_exit=no'

output=$(run_program "$exercise_dir/11_waitpid_specific_child")
assert_contains "$output" 'first_status=22 second_status=11 specific_order=yes'

output=$(run_program "$exercise_dir/12_waitpid_nohang")
assert_contains "$output" 'initial_running=yes final_exit=7'

output=$(run_program "$exercise_dir/13_waitid_wnowait")
assert_contains "$output" 'observed_pid_matches=yes observed_status=33 reaped_same=yes'

output=$(run_program "$exercise_dir/14_process_credentials")
assert_contains "$output" 'supplementary_groups='
assert_contains "$output" 'same_user_identity='

output=$(run_program "$exercise_dir/15_process_group")
assert_contains "$output" 'child_group_equals_child=yes parent_group_unchanged=yes'

output=$(run_program "$exercise_dir/16_new_session")
assert_contains "$output" 'pid_equals_pgid=yes pid_equals_sid=yes session_created=yes'

output=$(run_program "$exercise_dir/17_safe_command_runner")
assert_contains "$output" 'output=hello; echo injected shell_interpreted=no status=0'

output=$(run_program "$exercise_dir/18_daemon_session_setup")
assert_contains "$output" 'session_leader=yes process_group_leader=yes cwd_root=yes'
assert_contains "$output" 'standard_streams_redirected=yes child_exit=0'

output=$(run_program "$exercise_dir/19_reap_all_children")
assert_contains "$output" 'created=3 reaped=3 status_sum=33 all_collected=yes'

output=$(run_program "$experiment_dir/process_tree")
assert_contains "$output" 'child_parent_matches=yes grandchild_parent_matches=yes hierarchy_depth=2'

output=$(run_program "$experiment_dir/stdio_fork_duplication")
assert_contains "$output" 'buffered_before_fork=yes copies_in_file=2 duplicated=yes'

output=$(run_program "$experiment_dir/cow_page_faults")
assert_contains "$output" 'pages_written=128'
assert_contains "$output" 'fault_delta_positive=yes'
assert_contains "$output" 'parent_unchanged=yes'

output=$(run_program "$experiment_dir/zombie_lifecycle")
assert_contains "$output" 'exit_status=17 proc_entry_removed=yes'

output=$(run_program "$experiment_dir/subreaper_adoption")
if [[ $output == *'subreaper_supported=yes'* ]]; then
    assert_contains "$output" 'adopted_by_subreaper=yes'
    assert_contains "$output" 'grandchild_exit=17'
else
    assert_contains "$output" 'subreaper_supported=no'
fi

output=$(run_program "$experiment_dir/pid_runtime_context")
assert_contains "$output" 'pid_max='
assert_contains "$output" 'pid1_comm='
assert_contains "$output" 'runtime_values_used=yes'

output=$(run_program "$experiment_dir/wait4_usage")
assert_contains "$output" 'exit_status=9'
assert_contains "$output" 'usage_collected=yes'

printf 'PASS: all deterministic Chapter 5 checks completed\n'
