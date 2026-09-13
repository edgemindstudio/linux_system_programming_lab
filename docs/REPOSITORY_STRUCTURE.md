# Repository Structure

## Design Goal

The repository is organized by book chapter so each learning unit owns its
notes, code, tests, scripts, and evidence. The structure remains understandable
when all eleven chapters are implemented.

## Ownership Rules

### chapters/

Chapter-specific work lives inside its numbered chapter directory. A filename
does not repeat the chapter number because its parent directory already provides
that context.

Example:

~~~text
chapters/02-file-io/exercises/08_robust_copy.c
~~~

### foundation/

Repository bootstrap material that is not a book chapter lives here. This
includes the initial compiler smoke test and its original system-call traces.

### projects/

Programs that combine ideas from multiple chapters live here. Each project
should eventually own its source, tests, documentation, and observations.

### scripts/

Only repository-wide helpers live here. Chapter-specific helpers remain inside
the chapter that owns them.

### build/

All generated binaries and runtime data live here. The layout mirrors the
source hierarchy:

~~~text
source:
chapters/02-file-io/exercises/01_standard_fds.c

binary:
build/chapters/02-file-io/exercises/01_standard_fds
~~~

The build directory is ignored by Git and removed by make clean.

## Naming Rules

- Chapter directories use two digits and a descriptive lowercase slug.
- Exercise filenames start with a two-digit learning order.
- Experiment filenames describe the question or mechanism.
- Tests use descriptive shell-script names such as smoke.sh.
- Generated data never lives beside source code.
- Chapter paths are written relative to the repository root in commands.

## Supported Commands

~~~bash
make
make chapter CHAPTER=02
make chapter02
make test CHAPTER=02
make test-all
make tidy
make tidy-chapter CHAPTER=02
make sanitize
make check-structure
make clean
~~~

## Starting a Future Chapter

1. Change the chapter README status to Reading.
2. Add only the directories needed for actual work.
3. Begin with an original mental model and notes.
4. Add numbered exercises and descriptive experiments.
5. Store generated data under the matching build chapter directory.
6. Add a chapter test script.
7. Run the structure check, strict build, and tests.
8. Commit the chapter learning unit after review.

Do not create a second set of root-level exercises, notes, or tests. The chapter
directory remains the single owner.
