CC := clang

CPPFLAGS :=
CFLAGS := \
	-std=c17 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wconversion \
	-Wshadow \
	-Wformat=2 \
	-Wundef \
	-g3 \
	-O0

LDFLAGS :=
LDLIBS :=

BUILD_DIR := build
CHAPTER ?= 02

CHAPTER_DIRS := $(patsubst %/,%,$(dir $(wildcard chapters/*/README.md)))
FOUNDATION_SOURCES := $(sort $(wildcard foundation/*.c))
EXERCISE_SOURCES := $(sort $(wildcard chapters/*/exercises/*.c))
EXPERIMENT_SOURCES := $(sort $(wildcard chapters/*/experiments/*.c))
CHAPTER_SOURCES := $(EXERCISE_SOURCES) $(EXPERIMENT_SOURCES)
ALL_C_SOURCES := $(FOUNDATION_SOURCES) $(CHAPTER_SOURCES)

FOUNDATION_TARGETS := $(patsubst %.c,$(BUILD_DIR)/%,$(FOUNDATION_SOURCES))
CHAPTER_TARGETS := $(patsubst %.c,$(BUILD_DIR)/%,$(CHAPTER_SOURCES))
ALL_TARGETS := $(FOUNDATION_TARGETS) $(CHAPTER_TARGETS)

ACTIVE_CHAPTER_DIRS := $(sort \
	$(foreach source,$(CHAPTER_SOURCES), \
		$(patsubst %/,%,$(dir $(patsubst %/,%,$(dir $(source)))))))
ALL_DATA_DIRS := $(addprefix $(BUILD_DIR)/,$(addsuffix /data,$(ACTIVE_CHAPTER_DIRS)))

SELECTED_CHAPTER_DIR := $(firstword $(wildcard chapters/$(CHAPTER)-*))
SELECTED_SOURCES := \
	$(sort \
		$(wildcard $(SELECTED_CHAPTER_DIR)/exercises/*.c) \
		$(wildcard $(SELECTED_CHAPTER_DIR)/experiments/*.c))
SELECTED_TARGETS := $(patsubst %.c,$(BUILD_DIR)/%,$(SELECTED_SOURCES))
SELECTED_DATA_DIR := $(BUILD_DIR)/$(SELECTED_CHAPTER_DIR)/data
SELECTED_TESTS := $(sort $(wildcard $(SELECTED_CHAPTER_DIR)/tests/*.sh))
ALL_TESTS := $(sort $(wildcard chapters/*/tests/*.sh))
KNOWN_DATA_DIRS := $(sort $(ALL_DATA_DIRS) $(SELECTED_DATA_DIR))

CHAPTER_NUMBERS := 01 02 03 04 05 06 07 08 09 10 11
CHAPTER_ALIASES := $(addprefix chapter,$(CHAPTER_NUMBERS))

.PHONY: all foundation chapters chapter $(CHAPTER_ALIASES) run test test-all \
	tidy tidy-chapter sanitize list check-structure clean help

all: foundation chapters

foundation: $(FOUNDATION_TARGETS)

chapters: $(ALL_DATA_DIRS) $(CHAPTER_TARGETS)

$(BUILD_DIR)/%: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

$(KNOWN_DATA_DIRS):
	@mkdir -p $@

ifeq ($(strip $(SELECTED_CHAPTER_DIR)),)
chapter:
	@printf 'error: no chapter directory matches CHAPTER=%s\n' '$(CHAPTER)' >&2
	@exit 2

test:
	@printf 'error: no chapter directory matches CHAPTER=%s\n' '$(CHAPTER)' >&2
	@exit 2

tidy-chapter:
	@printf 'error: no chapter directory matches CHAPTER=%s\n' '$(CHAPTER)' >&2
	@exit 2
else
chapter: $(SELECTED_DATA_DIR) $(SELECTED_TARGETS)
	@printf 'Built Chapter %s from %s\n' '$(CHAPTER)' '$(SELECTED_CHAPTER_DIR)'

test: chapter
	@if [ -z '$(strip $(SELECTED_TESTS))' ]; then \
		printf 'No tests exist yet for Chapter %s.\n' '$(CHAPTER)'; \
	else \
		set -e; \
		for script in $(SELECTED_TESTS); do \
			printf 'Running %s\n' "$$script"; \
			bash "$$script"; \
		done; \
	fi

tidy-chapter:
	@if [ -z '$(strip $(SELECTED_SOURCES))' ]; then \
		printf 'No C sources exist yet for Chapter %s.\n' '$(CHAPTER)'; \
	else \
		clang-tidy $(SELECTED_SOURCES) -- $(CPPFLAGS) $(CFLAGS); \
	fi
endif

$(CHAPTER_ALIASES):
	@$(MAKE) --no-print-directory chapter CHAPTER=$(patsubst chapter%,%,$@)

run: $(BUILD_DIR)/foundation/smoke_test
	./$(BUILD_DIR)/foundation/smoke_test

test-all: chapters
	@if [ -z '$(strip $(ALL_TESTS))' ]; then \
		printf 'No chapter tests exist yet.\n'; \
	else \
		set -e; \
		for script in $(ALL_TESTS); do \
			printf 'Running %s\n' "$$script"; \
			bash "$$script"; \
		done; \
	fi

tidy:
	clang-tidy $(ALL_C_SOURCES) -- $(CPPFLAGS) $(CFLAGS)

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS='$(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer' all

list:
	@printf '%s\n' $(ALL_TARGETS)

check-structure:
	bash scripts/verify_structure.sh

clean:
	rm -rf $(BUILD_DIR)

help:
	@printf '%s\n' \
		'make                         Build the foundation and every implemented chapter' \
		'make chapter CHAPTER=02      Build one chapter by its two-digit number' \
		'make chapter02               Convenience alias for Chapter 02' \
		'make test CHAPTER=02         Build and test one chapter' \
		'make test-all                Build and test every implemented chapter' \
		'make run                     Build and run the foundation smoke test' \
		'make tidy                    Run clang-tidy over every C source' \
		'make tidy-chapter CHAPTER=02 Run clang-tidy over one chapter' \
		'make sanitize                Rebuild everything with ASan and UBSan' \
		'make list                    List all generated executables' \
		'make check-structure         Validate the chapter directory contract' \
		'make clean                   Remove generated build output' \
		'make help                    Display these targets'
