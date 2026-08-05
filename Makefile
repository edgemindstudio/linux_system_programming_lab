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
TARGET := $(BUILD_DIR)/smoke_test
SOURCE := src/smoke_test.c

.PHONY: all run clean help

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCE) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

help:
	@printf '%s\n' \
		'make       Build the smoke-test program' \
		'make run   Build and run the smoke test' \
		'make clean Remove generated build files' \
		'make help  Display available targets'
