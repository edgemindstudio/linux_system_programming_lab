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

if find chapters/02-file-io -type f -name 'chapter02_*' -print -quit |
    grep -q .
then
    printf 'FAIL: a Chapter 2 filename repeats its parent chapter number\n' >&2
    exit 1
fi

if [[ -n $(git ls-files build) ]]; then
    printf 'FAIL: generated build output is tracked by Git\n' >&2
    exit 1
fi

printf 'PASS: chapter-oriented repository structure is valid\n'
