#!/usr/bin/env bash

set -euo pipefail

repository_root=$(git rev-parse --show-toplevel)
cd "$repository_root"

expected_chapters=(
    01-introduction-and-essential-concepts
    02-file-io
    03-buffered-io
    04-advanced-file-io
    05-process-management
    06-advanced-process-management
    07-threading
    08-file-and-directory-management
    09-memory-management
    10-signals
    11-time
)

legacy_directories=(
    exercises
    experiments
    mental_maps
    notes
    observations
    src
    tests
)

for chapter in "${expected_chapters[@]}"
do
    if [[ ! -f "chapters/$chapter/README.md" ]]; then
        printf 'FAIL: missing chapters/%s/README.md\n' "$chapter" >&2
        exit 1
    fi
done

for directory in "${legacy_directories[@]}"
do
    if [[ -e $directory ]]; then
        printf 'FAIL: legacy root directory still exists: %s\n' "$directory" >&2
        exit 1
    fi
done

required_chapter02_paths=(
    chapters/02-file-io/notes/study-guide-pages-31-61.md
    chapters/02-file-io/mental-models/file-io.md
    chapters/02-file-io/exercises/01_standard_fds.c
    chapters/02-file-io/exercises/12_select_stdin.c
    chapters/02-file-io/experiments/direct_io.c
    chapters/02-file-io/experiments/sparse_file.c
    chapters/02-file-io/tests/smoke.sh
    chapters/02-file-io/scripts/trace.sh
)

for path in "${required_chapter02_paths[@]}"
do
    if [[ ! -f $path ]]; then
        printf 'FAIL: required Chapter 2 path is missing: %s\n' "$path" >&2
        exit 1
    fi
done

required_chapter03_paths=(
    chapters/03-buffered-io/notes/study-guide-pages-67-90.md
    chapters/03-buffered-io/mental-models/buffered-io.md
    chapters/03-buffered-io/exercises/01_standard_streams.c
    chapters/03-buffered-io/exercises/14_manual_stream_locking.c
    chapters/03-buffered-io/experiments/block_size.c
    chapters/03-buffered-io/experiments/threaded_records.c
    chapters/03-buffered-io/tests/smoke.sh
    chapters/03-buffered-io/scripts/trace.sh
)

for path in "${required_chapter03_paths[@]}"
do
    if [[ ! -f $path ]]; then
        printf 'FAIL: required Chapter 3 path is missing: %s\n' "$path" >&2
        exit 1
    fi
done

required_chapter04_paths=(
    chapters/04-advanced-file-io/notes/study-guide-pages-91-135.md
    chapters/04-advanced-file-io/mental-models/advanced-file-io.md
    chapters/04-advanced-file-io/exercises/01_writev_record.c
    chapters/04-advanced-file-io/exercises/16_inode_order.c
    chapters/04-advanced-file-io/experiments/linear_vs_vectored.c
    chapters/04-advanced-file-io/experiments/scheduler_inventory.c
    chapters/04-advanced-file-io/tests/smoke.sh
    chapters/04-advanced-file-io/scripts/trace.sh
)

for path in "${required_chapter04_paths[@]}"
do
    if [[ ! -f $path ]]; then
        printf 'FAIL: required Chapter 4 path is missing: %s\n' "$path" >&2
        exit 1
    fi
done

required_chapter05_paths=(
    chapters/05-process-management/notes/study-guide-pages-137-175.md
    chapters/05-process-management/mental-models/process-management.md
    chapters/05-process-management/exercises/01_process_identity.c
    chapters/05-process-management/exercises/19_reap_all_children.c
    chapters/05-process-management/experiments/process_tree.c
    chapters/05-process-management/experiments/subreaper_adoption.c
    chapters/05-process-management/tests/smoke.sh
    chapters/05-process-management/scripts/trace.sh
)

for path in "${required_chapter05_paths[@]}"
do
    if [[ ! -f $path ]]; then
        printf 'FAIL: required Chapter 5 path is missing: %s\n' "$path" >&2
        exit 1
    fi
done

required_chapter06_paths=(
    chapters/06-advanced-process-management/notes/study-guide-pages-177-209.md
    chapters/06-advanced-process-management/mental-models/advanced-process-management.md
    chapters/06-advanced-process-management/exercises/01_scheduler_snapshot.c
    chapters/06-advanced-process-management/exercises/18_mlock_one_page.c
    chapters/06-advanced-process-management/experiments/cpu_io_bound.c
    chapters/06-advanced-process-management/experiments/proc_sched_inventory.c
    chapters/06-advanced-process-management/tests/smoke.sh
    chapters/06-advanced-process-management/scripts/trace.sh
)

for path in "${required_chapter06_paths[@]}"
do
    if [[ ! -f $path ]]; then
        printf 'FAIL: required Chapter 6 path is missing: %s\n' "$path" >&2
        exit 1
    fi
done

required_chapter07_paths=(
    chapters/07-threading/notes/study-guide-pages-211-239.md
    chapters/07-threading/mental-models/threading.md
    chapters/07-threading/exercises/01_thread_identity.c
    chapters/07-threading/exercises/18_condition_handoff.c
    chapters/07-threading/experiments/logical_race_atomic.c
    chapters/07-threading/experiments/deadlock_timeout.c
    chapters/07-threading/tests/smoke.sh
    chapters/07-threading/scripts/trace.sh
)

for path in "${required_chapter07_paths[@]}"
do
    if [[ ! -f $path ]]; then
        printf 'FAIL: required Chapter 7 path is missing: %s\n' "$path" >&2
        exit 1
    fi
done

if find chapters/02-file-io -type f -name 'chapter02_*' -print -quit |
    grep -q .
then
    printf 'FAIL: a Chapter 2 filename repeats its parent chapter number\n' >&2
    exit 1
fi

if find chapters/03-buffered-io -type f -name 'chapter03_*' -print -quit |
    grep -q .
then
    printf 'FAIL: a Chapter 3 filename repeats its parent chapter number\n' >&2
    exit 1
fi

if find chapters/04-advanced-file-io -type f -name 'chapter04_*' -print -quit |
    grep -q .
then
    printf 'FAIL: a Chapter 4 filename repeats its parent chapter number\n' >&2
    exit 1
fi

if find chapters/05-process-management -type f -name 'chapter05_*' -print -quit |
    grep -q .
then
    printf 'FAIL: a Chapter 5 filename repeats its parent chapter number\n' >&2
    exit 1
fi

if find chapters/06-advanced-process-management -type f -name 'chapter06_*' -print -quit |
    grep -q .
then
    printf 'FAIL: a Chapter 6 filename repeats its parent chapter number\n' >&2
    exit 1
fi

if find chapters/07-threading -type f -name 'chapter07_*' -print -quit |
    grep -q .
then
    printf 'FAIL: a Chapter 7 filename repeats its parent chapter number\n' >&2
    exit 1
fi

if [[ -n $(git ls-files build) ]]; then
    printf 'FAIL: generated build output is tracked by Git\n' >&2
    exit 1
fi

printf 'PASS: chapter-oriented repository structure is valid\n'
