#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel)
exercise_dir="$repository_root/build/chapters/11-time/exercises"
experiment_dir="$repository_root/build/chapters/11-time/experiments"

expect_text()
{
    local executable=$1
    local expected=$2
    local output

    output=$(timeout 5 "$executable")
    if ! grep -Fq -- "$expected" <<<"$output"; then
        printf 'FAIL: %s did not contain: %s\n' "$executable" "$expected" >&2
        printf '%s\n' "$output" >&2
        exit 1
    fi
}

expect_pattern()
{
    local executable=$1
    local pattern=$2
    local output

    output=$(timeout 5 "$executable")
    if ! grep -Eq -- "$pattern" <<<"$output"; then
        printf 'FAIL: %s did not match: %s\n' "$executable" "$pattern" >&2
        printf '%s\n' "$output" >&2
        exit 1
    fi
}

expect_text "$exercise_dir/01_time_data_types" "sizes_positive=yes"
expect_text "$exercise_dir/02_time_epoch_seconds" "returned_matches_stored=yes"
expect_text "$exercise_dir/03_gettimeofday_microseconds" "microseconds_in_range=yes"
expect_text "$exercise_dir/04_realtime_and_monotonic" "monotonic_valid=yes"
expect_text "$exercise_dir/05_clock_resolution" "monotonic_resolution_positive=yes"
expect_text "$exercise_dir/06_timespec_arithmetic" "expected_1_2_seconds=yes"
expect_text "$exercise_dir/07_process_cpu_clock" "process_cpu_elapsed_nonnegative=yes"
expect_text "$exercise_dir/08_thread_cpu_clock" "thread_cpu_elapsed_nonnegative=yes"
expect_text "$exercise_dir/09_times_process_accounting" "ticks_per_second_positive=yes"
expect_text "$exercise_dir/10_utc_and_local_time" "same_epoch_input=yes"
expect_text "$exercise_dir/11_strftime_format" "expected_text=yes"
expect_text "$exercise_dir/12_mktime_roundtrip" "roundtrip_matches=yes"
expect_text "$exercise_dir/13_nanosleep_elapsed" "elapsed_at_least_requested=yes"
expect_text "$exercise_dir/14_nanosleep_interrupted" "errno_is_eintr=yes"
expect_text "$exercise_dir/15_clock_nanosleep_absolute" "reached_deadline=yes"
expect_text "$exercise_dir/16_alarm_one_shot" "alarm_delivered=yes"
expect_text "$exercise_dir/17_setitimer_one_shot" "expired=yes"
expect_text "$exercise_dir/18_posix_timer" "payload_matches=yes"

expect_text "$experiment_dir/wall_vs_cpu_time" "blocked_time_is_not_cpu_time=yes"
expect_text "$experiment_dir/realtime_monotonic_samples" "monotonic_non_decreasing=yes"
expect_text "$experiment_dir/relative_vs_absolute_drift" "absolute_schedule_uses_fixed_deadlines=yes"
expect_text "$experiment_dir/timer_overrun" "overrun_nonnegative=yes"
expect_text "$experiment_dir/timerfd_expirations" "expirations_positive=yes"
expect_text "$experiment_dir/boottime_comparison" "samples_within_five_seconds=yes"
expect_pattern "$experiment_dir/clocksource_inventory" \
    'environment_dependent=yes|kernel_selects_clocksource=yes'

printf 'PASS: all deterministic Chapter 11 checks completed\n'
