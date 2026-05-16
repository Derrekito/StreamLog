# This Makefile compiles the 'streamlog_example' binary using the g++ compiler.
# The source files are located in the 'src' directory and the binary will be
# generated in the 'build' directory. The Makefile includes flags for
# specifying include directories, macro definitions, and warnings. It also
# handles linker flags and libraries for Boost. The user can set the desired
# debug level with the 'DEBUG_LEVEL' variable and color theme with the 'THEME'
# variable. The Makefile provides 'all', 'clean', 'distclean', and 'install'
# targets to build, clean, and install the outputs.

####################################################
# DEBUG LEVELS
####################################################
# Lower levels encompass higher levels.
# 1 TRACE
# 2 DEBUG
# 3 INFO
# 4 WARN
# 5 ERROR
# 6 FATAL

####################################################
# build switches
####################################################
aarch64 ?= 0
mac ?= 0

####################################################
# includes
####################################################
include mk/aarch64.mk
include mk/example.mk
include mk/helper.mk

####################################################
# Set compiler
####################################################
CXX ?= g++

####################################################
# Installation paths
####################################################
PREFIX  ?= /usr/local
DESTDIR ?=

####################################################
# Output directories
####################################################
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
BIN_DIR   := $(BUILD_DIR)/bin
LIB_DIR   := $(BUILD_DIR)/lib

####################################################
# Compiler flags
####################################################
# Initialize flags for include directories, macro definitions, and warnings
INCLUDE_FLAGS  ?=
MACRO_FLAGS    ?=
WARNING_FLAGS  ?=
EXTRA_FLAGS    ?= --std=c++11

# Assemble the compilation flags
CFLAGS   = $(INCLUDE_FLAGS) $(MACRO_FLAGS) $(WARNING_FLAGS) $(EXTRA_FLAGS)
LDFLAGS ?=
LDLIBS  ?=

####################################################
# Project paths
####################################################
SRC_PATH := src
INCLUDE_PATH := include

# Add include directories
INCLUDE_FLAGS += -I$(INCLUDE_PATH)

# Add warning flags
WARNING_FLAGS += -Wall -Wextra

# Set the debug level
DEBUG_LEVEL ?= 1
MACRO_FLAGS += -DDEBUG_LEVEL=$(DEBUG_LEVEL)

####################################################
# Theme
####################################################
THEME ?= default
ifeq ($(THEME),rose_pine_moon)
    MACRO_FLAGS += -DSTREAMLOG_THEME_ROSE_PINE_MOON
endif

# Enable specific logging
MACRO_FLAGS += -DENABLE_VECTOR_LOGGING
MACRO_FLAGS += -DENABLE_MAP_LOGGING

####################################################
# Source and object files
####################################################
# Define the source files
SRC_FILES := $(SRC_PATH)/streamlog.cpp

# Set the target library name
LIB_TARGET := streamlog

# Define the object files
CORE_OBJ := $(OBJ_DIR)/$(notdir $(SRC_FILES:.cpp=.o))

####################################################
# Library targets
####################################################
LIB_STATIC  := $(LIB_DIR)/lib$(LIB_TARGET).a
LIB_DYNAMIC := $(LIB_DIR)/lib$(LIB_TARGET).so
ifeq ($(mac),1)
	LIB_MAC := $(LIB_DIR)/lib$(LIB_TARGET).dylib
endif

# Define all library targets
LIB_ALL := $(LIB_STATIC) $(LIB_DYNAMIC)

# Add the dynamic library target if on macOS
ifeq ($(mac),1)
	LIB_ALL += $(LIB_MAC)
endif

####################################################
# Test configuration
####################################################
TEST_DIR := tests
TEST_SRC := $(TEST_DIR)/test_streamlog.cpp
TEST_BIN := $(BIN_DIR)/test_streamlog
TEST_BIN_ASAN := $(BIN_DIR)/test_streamlog_asan
TEST_BIN_UBSAN := $(BIN_DIR)/test_streamlog_ubsan

####################################################
# Recipes
####################################################
# Declare phony targets to avoid conflicts with files of the same name
.PHONY: all clean distclean install prepare docs test test-asan test-ubsan test-memcheck test-all

# Default target
all: $(LIB_ALL)

# Prepare build directories
prepare:
	@$(MKDIR) $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)

# Compile the object file
$(OBJ_DIR)/%.o: $(SRC_PATH)/%.cpp | prepare
	$(CXX) $(CFLAGS) -c $< -o $@

# Compile the static library
$(LIB_STATIC): $(CORE_OBJ) | prepare
	@$(AR) rcs $@ $^

# Compile the shared library (recompile with -fPIC)
$(LIB_DYNAMIC): $(SRC_FILES) | prepare
	$(CXX) -fPIC -shared $(CFLAGS) -o $@ $^

# Compile the dynamic library on macOS
ifeq ($(mac),1)
$(LIB_MAC): $(SRC_FILES) | prepare
	$(CXX) -dynamiclib -fPIC $(CFLAGS) -o $@ $^
endif

# Clean up build artifacts
clean:
	@echo "clean"
	@$(RM) -r $(BUILD_DIR)

# Extended cleanup
distclean: clean
	@$(RM) *~ *.swp *.bak *.tmp
	@$(RM) -r docs test_logs
	@$(RM) -f test_custom.log output.log

# Generate documentation
docs:
	@command -v doxygen >/dev/null || { echo "Error: doxygen not found"; exit 1; }
	@echo "Generating documentation..."
	@doxygen Doxyfile
	@echo "Documentation generated in docs/html/index.html"

# Build and run tests
test: $(TEST_BIN)
	@echo "Running unit tests..."
	@LD_LIBRARY_PATH=$(LIB_DIR):$$LD_LIBRARY_PATH $(TEST_BIN)

# Compile test binary
$(TEST_BIN): $(TEST_SRC) $(LIB_DYNAMIC) | prepare
	$(CXX) $(CFLAGS) -I$(TEST_DIR) $(TEST_SRC) -L$(LIB_DIR) -l$(LIB_TARGET) -o $(TEST_BIN)

# AddressSanitizer - detects memory errors (use-after-free, buffer overflow, leaks)
test-asan: $(TEST_BIN_ASAN)
	@echo "Running tests with AddressSanitizer..."
	@LD_LIBRARY_PATH=$(LIB_DIR):$$LD_LIBRARY_PATH ASAN_OPTIONS=detect_leaks=1 $(TEST_BIN_ASAN)

$(TEST_BIN_ASAN): $(TEST_SRC) $(LIB_DYNAMIC) | prepare
	$(CXX) $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer -g -I$(TEST_DIR) $(TEST_SRC) -L$(LIB_DIR) -l$(LIB_TARGET) -o $(TEST_BIN_ASAN)

# UndefinedBehaviorSanitizer - detects undefined behavior
test-ubsan: $(TEST_BIN_UBSAN)
	@echo "Running tests with UndefinedBehaviorSanitizer..."
	@LD_LIBRARY_PATH=$(LIB_DIR):$$LD_LIBRARY_PATH $(TEST_BIN_UBSAN)

$(TEST_BIN_UBSAN): $(TEST_SRC) $(LIB_DYNAMIC) | prepare
	$(CXX) $(CFLAGS) -fsanitize=undefined -fno-omit-frame-pointer -g -I$(TEST_DIR) $(TEST_SRC) -L$(LIB_DIR) -l$(LIB_TARGET) -o $(TEST_BIN_UBSAN)

# Memory leak check with Valgrind (if available)
test-memcheck: $(TEST_BIN)
	@if command -v valgrind >/dev/null 2>&1; then \
		echo "Running tests with Valgrind memcheck..."; \
		LD_LIBRARY_PATH=$(LIB_DIR):$$LD_LIBRARY_PATH valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 $(TEST_BIN); \
	else \
		echo "Valgrind not found, skipping memcheck (install with: sudo pacman -S valgrind)"; \
		exit 1; \
	fi

# Run all test variations
test-all: test test-asan test-ubsan
	@echo ""
	@echo "=== All Tests Passed ==="
	@echo "✓ Standard tests"
	@echo "✓ AddressSanitizer (memory errors)"
	@echo "✓ UndefinedBehaviorSanitizer"

# Install rule for the library
install: $(LIB_DYNAMIC) $(LIB_STATIC)
	@if [ -z "$(DESTDIR)" ]; then \
		target="$(PREFIX)"; \
		while [ ! -e "$$target" ] && [ "$$target" != "/" ]; do \
			target=$$(dirname "$$target"); \
		done; \
		if [ ! -w "$$target" ]; then \
			echo "Error: No write permission to $(PREFIX)"; \
			echo "Try: sudo make install"; \
			echo "  or: make install PREFIX=~/.local"; \
			exit 1; \
		fi; \
	fi
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 644 $(LIB_DYNAMIC) $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(LIB_STATIC) $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(INCLUDE_PATH)/streamlog.hpp $(DESTDIR)$(PREFIX)/include/
	@command -v ldconfig >/dev/null && ldconfig || true
