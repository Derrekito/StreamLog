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

# Add include directories
INCLUDE_FLAGS += -I$(SRC_PATH)

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
LIB_STATIC  := $(LIB_DIR)/$(LIB_TARGET).a
LIB_DYNAMIC := $(LIB_DIR)/$(LIB_TARGET).so
ifeq ($(mac),1)
	LIB_MAC := $(LIB_DIR)/$(LIB_TARGET).dylib
endif

# Define all library targets
LIB_ALL := $(LIB_STATIC) $(LIB_DYNAMIC)

# Add the dynamic library target if on macOS
ifeq ($(mac),1)
	LIB_ALL += $(LIB_MAC)
endif

####################################################
# Recipes
####################################################
# Declare phony targets to avoid conflicts with files of the same name
.PHONY: all clean distclean install prepare

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
	install -m 644 $(SRC_PATH)/streamlog.hpp $(DESTDIR)$(PREFIX)/include/
	@command -v ldconfig >/dev/null && ldconfig || true
