# List of test binaries
DECORATOR_TARGET := dectorator_test

TEST_TARGETS := $(DECORATOR_TARGET)
TEST_PATH := tests

# Decorator test build info
DECORATOR_SRC := decorator.cpp
DECORATOR_FLAGS := -ggdb3 -fsanitize=address
DECORATOR_SRCS := $(patsubst %, $(TEST_PATH)/%, $(DECORATOR_SRC))