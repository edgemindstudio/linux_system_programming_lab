#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
cd "$repository_root"

exercise_dir=build/chapters/08-file-and-directory-management/exercises
experiment_dir=build/chapters/08-file-and-directory-management/experiments

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

output=$(run_program "$exercise_dir/01_stat_metadata")
assert_contains "$output" 'regular=yes size=9'
assert_contains "$output" 'inode_positive=yes size_matches=yes metadata_is_snapshot=yes'

output=$(run_program "$exercise_dir/02_stat_vs_lstat")
assert_contains "$output" 'stat_follows_link=yes lstat_reports_link=yes'
assert_contains "$output" 'target_and_link_inodes_distinct=yes'

output=$(run_program "$exercise_dir/03_file_type_mode")
assert_contains "$output" 'regular_detected=yes directory_detected=yes fifo_detected=yes'

output=$(run_program "$exercise_dir/04_fchmod_permissions")
assert_contains "$output" 'requested=0640 observed=0640'
assert_contains "$output" 'permissions_changed=yes descriptor_target_stable=yes'

output=$(run_program "$exercise_dir/05_ownership_and_access")
assert_contains "$output" 'owned_by_real_user=yes'
assert_contains "$output" 'readable_writable=yes'

output=$(run_program "$exercise_dir/06_extended_attributes")
if [[ $output == *'xattr_supported=yes'* ]]; then
    assert_contains "$output" 'value=metadata-lab listed=yes'
    assert_contains "$output" 'removed=yes inode_metadata_extended=yes'
else
    assert_contains "$output" 'xattr_supported=no'
fi

output=$(run_program "$exercise_dir/07_current_working_directory")
assert_contains "$output" 'directory_changed=yes directory_restored=yes'
assert_contains "$output" 'descriptor_based_restore=yes'

output=$(run_program "$exercise_dir/08_mkdir_umask")
assert_contains "$output" 'requested=0777 umask=0027 observed=0750'
assert_contains "$output" 'effective_mode_correct=yes'

output=$(run_program "$exercise_dir/09_rmdir_nonempty")
assert_contains "$output" 'nonempty_removal_rejected=yes empty_removal_succeeded=yes'

output=$(run_program "$exercise_dir/10_read_directory")
assert_contains "$output" 'alpha=found beta=found gamma=found'
assert_contains "$output" 'all_expected_entries=yes order_assumed=no dtype_required=no'

output=$(run_program "$exercise_dir/11_hard_link")
assert_contains "$output" 'same_inode=yes first_links=2 second_links=2'
assert_contains "$output" 'two_names_one_inode=yes hard_link_not_copy=yes'

output=$(run_program "$exercise_dir/12_symbolic_link")
assert_contains "$output" 'stored_path=symlink_target.txt path_matches=yes'
assert_contains "$output" 'link_is_separate_inode=yes stat_followed_target=yes'

output=$(run_program "$exercise_dir/13_unlink_open_file")
assert_contains "$output" 'name_gone=yes link_count=0 data=still-readable'
assert_contains "$output" 'descriptor_kept_inode_alive=yes'

output=$(run_program "$exercise_dir/14_rename_file")
assert_contains "$output" 'old_name_gone=yes new_name_exists=yes same_inode=yes'
assert_contains "$output" 'namespace_changed_without_copy=yes'

output=$(run_program "$exercise_dir/15_copy_file")
assert_contains "$output" 'sizes_match=yes distinct_inodes=yes'
assert_contains "$output" 'mode_preserved=yes metadata_policy_explicit=yes'

output=$(run_program "$exercise_dir/16_random_device")
assert_contains "$output" 'character_device=yes bytes_read=32'
assert_contains "$output" 'random_values_not_printed=yes driver_served_read=yes'

output=$(run_program "$exercise_dir/17_inotify_create_event")
assert_contains "$output" 'watch_descriptor_nonnegative=yes create_event_found=yes'
assert_contains "$output" 'event_records_variable_length=yes inotify_is_fd_based=yes'

output=$(run_program "$exercise_dir/18_inotify_lifecycle")
assert_contains "$output" 'pending_bytes_positive=yes ignored_event_found=yes'
assert_contains "$output" 'watch_removed=yes instance_closed=yes'

output=$(run_program "$experiment_dir/open_fd_across_rename")
assert_contains "$output" 'descriptor_inode_matches_new_name=yes old_name_gone=yes'
assert_contains "$output" 'proc_fd_mentions_new_name=yes inode_identity_survived=yes'

output=$(run_program "$experiment_dir/link_count_lifecycle")
assert_contains "$output" 'link_counts=3,2,1'
assert_contains "$output" 'count_decremented=yes remaining_data_readable=yes'

output=$(run_program "$experiment_dir/directory_fd_stability")
assert_contains "$output" 'directory_inode_stable=yes cwd_uses_new_name=yes'

output=$(run_program "$experiment_dir/metadata_timestamps")
assert_contains "$output" 'atime_set=yes mtime_set=yes'
assert_contains "$output" 'ctime_is_status_change=yes ctime_is_creation_time=no'

output=$(run_program "$experiment_dir/symlink_nofollow")
assert_contains "$output" 'default_open_followed=yes nofollow_rejected_link=yes'

output=$(run_program "$experiment_dir/inotify_rename_cookie")
assert_contains "$output" 'moved_from_found=yes moved_to_found=yes'
assert_contains "$output" 'cookies_match=yes cookie_nonzero=yes'

output=$(run_program "$experiment_dir/inotify_nonrecursive")
assert_contains "$output" 'child_directory_event=yes nested_file_event=no'
assert_contains "$output" 'watch_is_nonrecursive=yes child_needs_own_watch=yes'

printf 'PASS: all deterministic Chapter 8 checks completed\n'
