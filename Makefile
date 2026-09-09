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
DATA_DIR := $(BUILD_DIR)/chapter02/data

SOURCE_FILES := $(wildcard src/*.c)
EXERCISE_FILES := $(wildcard exercises/chapter02_*.c)
EXPERIMENT_FILES := $(wildcard experiments/chapter02_*.c)

SOURCE_TARGETS := $(patsubst src/%.c,$(BUILD_DIR)/src/%,$(SOURCE_FILES))
EXERCISE_TARGETS := $(patsubst exercises/%.c,$(BUILD_DIR)/exercises/%,$(EXERCISE_FILES))
EXPERIMENT_TARGETS := $(patsubst experiments/%.c,$(BUILD_DIR)/experiments/%,$(EXPERIMENT_FILES))
CHAPTER02_TARGETS := $(EXERCISE_TARGETS) $(EXPERIMENT_TARGETS)

.PHONY: all smoke exercises experiments chapter02 run run-smoke test tidy \
	sanitize list clean help

all: smoke chapter02

smoke: $(SOURCE_TARGETS)

exercises: $(EXERCISE_TARGETS)

experiments: $(EXPERIMENT_TARGETS)

chapter02: $(CHAPTER02_TARGETS) | $(DATA_DIR)

$(BUILD_DIR)/src/%: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/exercises/%: exercises/%.c | $(DATA_DIR)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/experiments/%: experiments/%.c | $(DATA_DIR)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

$(DATA_DIR):
	@mkdir -p $@

run: run-smoke

run-smoke: $(BUILD_DIR)/src/smoke_test
	./$(BUILD_DIR)/src/smoke_test

test: chapter02
	bash tests/chapter02_smoke.sh

tidy:
	clang-tidy $(SOURCE_FILES) $(EXERCISE_FILES) $(EXPERIMENT_FILES) -- \
		$(CPPFLAGS) $(CFLAGS)

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS='$(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer' all

list:
	@printf '%s\n' $(SOURCE_TARGETS) $(CHAPTER02_TARGETS)

clean:
	rm -rf $(BUILD_DIR)

help:
	@printf '%s\n' \
		'make             Build the smoke test and all Chapter 2 programs' \
		'make smoke       Build programs in src/' \
		'make chapter02   Build all Chapter 2 exercises and experiments' \
		'make exercises   Build Chapter 2 exercises only' \
		'make experiments Build Chapter 2 experiments only' \
		'make run         Build and run the smoke test' \
		'make test        Build and run deterministic Chapter 2 checks' \
		'make tidy        Run clang-tidy over C sources' \
		'make sanitize    Rebuild with AddressSanitizer and UBSan' \
		'make list        List all generated executable paths' \
		'make clean       Remove generated build files' \
		'make help        Display available targets'
