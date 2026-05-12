# Building

StreamLog uses a GNU Make-based build system designed for simplicity and cross-platform compatibility. The build produces both static and shared libraries, with optional platform-specific variants for macOS and embedded systems like NVIDIA Jetson. This document covers the complete build process, from prerequisites through installation, including platform-specific builds, build customization options, and the internal Makefile structure for developers who need to modify or extend the build system.

## Prerequisites

StreamLog has minimal dependencies, requiring only standard development tools available on most Unix-like systems. The library is written in C++11 and requires a compiler that supports this standard or later. No external libraries are required for the core functionality, though optional features may require additional packages. Before building StreamLog, ensure your system has the following tools installed.

- C++11 compatible compiler (GCC 4.8+, Clang 3.3+)
- GNU Make
- Standard C++ library

## Quick Build

```bash
git clone https://github.com/Derrekito/streamlog.git
cd streamlog
make
```

Output files are created in `build/`:

```
build/
├── obj/
│   └── streamlog.o
└── lib/
    ├── streamlog.a    # Static library
    └── streamlog.so   # Shared library
```

## Platform-Specific Builds

StreamLog supports multiple target platforms through conditional Makefile includes that set appropriate compiler flags and output formats. The default build targets Linux systems and produces standard `.a` and `.so` libraries. Additional platform configurations handle macOS's different dynamic library format and embedded platforms like NVIDIA Jetson that may require cross-compilation or specialized toolchains. Select the appropriate platform by passing the corresponding flag to make.

### Linux (Default)

```bash
make
```

Creates `.a` (static) and `.so` (shared) libraries using the system default `g++` compiler. This is the standard configuration for most Linux development environments and CI/CD pipelines. The resulting libraries follow standard Linux conventions and can be installed system-wide or linked from the build directory.

### macOS

```bash
make mac=1
```

Creates `.a`, `.so`, and `.dylib` libraries. The `.dylib` is the native macOS dynamic library format, built with the `-dynamiclib` flag instead of `-shared`. While macOS can use `.so` files in some cases, applications expecting native macOS conventions require the `.dylib` format. Use this option when building on macOS or cross-compiling for macOS targets.

**What it does:**
- Adds `$(LIB_DIR)/streamlog.dylib` to the build targets
- Uses `-dynamiclib -fPIC` flags for the dynamic library
- Preserves standard `.a` and `.so` builds for compatibility

### ARM64 (aarch64)

```bash
make aarch64=1
```

Configures cross-compilation for ARM64/aarch64 platforms such as NVIDIA Jetson, Raspberry Pi 4/5, AWS Graviton, or any ARM64 Linux system. This option sets the compiler to the ARM64 cross-compilation toolchain, enabling builds on x86_64 development machines that target ARM64 hardware. For native builds directly on ARM64 hardware, the default Linux build usually works without this flag.

**What it does:**
- Sets `CXX` to `aarch64-linux-gnu-g++-9`
- Sets `AR` to `aarch64-linux-gnu-ar`
- Targets the aarch64 architecture

**Prerequisites for cross-compilation:**
```bash
sudo apt install g++-9-aarch64-linux-gnu
```

## Build Targets

The Makefile provides several targets for building, cleaning, and installing the library. The default `all` target builds both static and shared libraries, while utility targets handle directory preparation and cleanup. The `install` target copies the built libraries and headers to a configurable prefix, with support for DESTDIR staging for package builds.

| Target | Description |
|--------|-------------|
| `all` | Build static and shared libraries (default) |
| `prepare` | Create build directories |
| `clean` | Remove `build/` directory |
| `distclean` | Remove `build/` and temp files (`*~`, `*.swp`, etc.) |
| `install` | Install to `PREFIX` (default: `/usr/local`) |

## Build Variables

Build variables control compilation behavior and can be overridden on the command line when invoking make. These variables allow customization of the compiler, optimization level, debug settings, and installation paths without modifying the Makefile. All variables have sensible defaults suitable for typical Linux development environments. Override any variable by passing it as an argument to make.

```bash
make CXX=clang++ DEBUG_LEVEL=3 PREFIX=/opt/streamlog
```

| Variable | Default | Description |
|----------|---------|-------------|
| `CXX` | `g++` | C++ compiler |
| `DEBUG_LEVEL` | `1` | Minimum log level (1-6) |
| `PREFIX` | `/usr/local` | Installation prefix |
| `DESTDIR` | (empty) | Staging directory for packaging |
| `EXTRA_FLAGS` | `--std=c++11` | Additional compiler flags |

## Makefile Structure

The build system is organized into a main Makefile and modular include files that separate concerns and enable platform-specific customization. The main Makefile handles core logic, variable definitions, and target recipes, while platform-specific settings are isolated in separate `.mk` files under the `mk/` directory. This modular structure makes it straightforward to add new platform targets or modify existing ones without affecting the core build logic. The following overview shows the file organization and key sections.

```
streamlog/
├── Makefile        # Main build configuration
└── mk/
    ├── helper.mk   # Common utilities (RM, AR, MKDIR)
    ├── aarch64.mk  # ARM64 cross-compilation settings
    └── example.mk  # Example build configuration
```

### Main Makefile Sections

```makefile
####################################################
# Build switches
####################################################
aarch64 ?= 0
mac ?= 0

####################################################
# Includes
####################################################
include mk/aarch64.mk
include mk/example.mk
include mk/helper.mk

####################################################
# Compiler configuration
####################################################
CXX ?= g++

####################################################
# Output directories
####################################################
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
LIB_DIR   := $(BUILD_DIR)/lib

####################################################
# Compiler flags
####################################################
CFLAGS = $(INCLUDE_FLAGS) $(MACRO_FLAGS) $(WARNING_FLAGS) $(EXTRA_FLAGS)

####################################################
# Library targets
####################################################
LIB_STATIC  := $(LIB_DIR)/streamlog.a
LIB_DYNAMIC := $(LIB_DIR)/streamlog.so

####################################################
# Recipes
####################################################
all: $(LIB_ALL)
```

### helper.mk

Provides common build utilities:

```makefile
AWK     := awk
MKDIR   := mkdir -p
RM      := rm -f
AR      ?= ar
SHELL   ?= /bin/bash

HOST_ARCH := $(shell uname -m)
HOST_SIZE := $(shell uname -m | sed -e "s/x86_64/64/" ...)
```

### aarch64.mk

Cross-compilation settings for ARM64 targets:

```makefile
ifeq ($(aarch64),1)
    # ARM64 cross-compilation toolchain
    CXX := aarch64-linux-gnu-g++-9
    AR := aarch64-linux-gnu-ar
endif
```

## Compilation Flags

The build system uses several categories of compiler flags to control language standard, warnings, include paths, and preprocessor definitions. Understanding these flags helps when diagnosing build issues or customizing the compilation for specific requirements. The flags are organized into logical groups that can be individually overridden or extended. This section documents the default flags and explains the flags used for each library type.

### Default Flags

```makefile
EXTRA_FLAGS    ?= --std=c++11
WARNING_FLAGS  += -Wall -Wextra
INCLUDE_FLAGS  += -I$(SRC_PATH)
MACRO_FLAGS    += -DDEBUG_LEVEL=$(DEBUG_LEVEL)
MACRO_FLAGS    += -DENABLE_VECTOR_LOGGING -DENABLE_MAP_LOGGING
```

### Static Library

```bash
$(AR) rcs $@ $^
```

- `r` - Insert/replace files
- `c` - Create archive
- `s` - Write index (ranlib)

### Shared Library

```bash
$(CXX) -fPIC -shared $(CFLAGS) -o $@ $^
```

- `-fPIC` - Position-independent code (required for shared libs)
- `-shared` - Create shared object

### macOS Dynamic Library

```bash
$(CXX) -dynamiclib -fPIC $(CFLAGS) -o $@ $^
```

## Debug Builds

When developing applications that use StreamLog or debugging issues within the library itself, building with debug symbols enables meaningful stack traces and debugger integration. The build system supports various debug configurations through the EXTRA_FLAGS variable, including AddressSanitizer for detecting memory errors. Debug builds disable optimization to ensure source code lines correspond directly to executed instructions.

Build with debug symbols:

```bash
make EXTRA_FLAGS="-g -O0 --std=c++11"
```

Build with sanitizers:

```bash
make EXTRA_FLAGS="-g -fsanitize=address --std=c++11"
```

## Installation

The install target copies the built libraries and header files to the specified PREFIX directory, making them available system-wide or in a user-local location. The installation respects the DESTDIR variable for staged installs, commonly used when building packages for distribution. Libraries are installed to `$(PREFIX)/lib` and headers to `$(PREFIX)/include`, following standard Unix filesystem conventions. The following sections cover common installation scenarios.

### System-Wide

```bash
sudo make install
```

Installs to `/usr/local/`:

- `lib/streamlog.so`, `lib/streamlog.a`
- `include/streamlog.hpp`

### User-Local

```bash
make install PREFIX=~/.local
```

Then add to your environment:

```bash
export LD_LIBRARY_PATH=~/.local/lib:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=~/.local/include:$CPLUS_INCLUDE_PATH
```

### Package Staging

For package builds (rpm, deb):

```bash
make install DESTDIR=/tmp/streamlog-pkg PREFIX=/usr
```

Creates:

```
/tmp/streamlog-pkg/
└── usr/
    ├── lib/
    │   ├── streamlog.a
    │   └── streamlog.so
    └── include/
        └── streamlog.hpp
```

## Adding a New Platform

The modular build system makes it straightforward to add support for new target platforms. Each platform gets its own `.mk` file that conditionally sets compiler paths, architecture-specific flags, and any other required build options. The platform is then activated via a command-line switch, keeping the main Makefile clean and platform-agnostic. Follow these steps to add support for a new platform.

1. Create `mk/newplatform.mk`:

```makefile
ifeq ($(newplatform),1)
    CXX := /path/to/cross-compiler
    EXTRA_FLAGS += -march=specific-arch
    # Platform-specific settings
endif
```

2. Add to main Makefile:

```makefile
newplatform ?= 0
include mk/newplatform.mk
```

3. Build:

```bash
make newplatform=1
```

## Troubleshooting Builds

Build errors typically fall into a few common categories: linker path issues, missing dependencies, or ABI incompatibilities. This section covers the most frequently encountered build problems and their solutions. Most issues can be resolved by ensuring proper library and include paths, or by rebuilding with consistent compiler settings. The following subsections address specific error messages and their resolutions.

### "cannot find -llog"

Library not in linker path. Either:

```bash
# Option 1: Install
sudo make install

# Option 2: Use build directory
g++ -L/path/to/streamlog/build/lib myapp.cpp -llog

# Option 3: Set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/path/to/streamlog/build/lib:$LD_LIBRARY_PATH
```

### Undefined Reference Errors

Ensure you're linking the library:

```bash
# Correct order: source files before libraries
g++ myapp.cpp -llog -o myapp
```

### ABI Compatibility

If linking against a library built with a different compiler, you may encounter ABI issues. Rebuild streamlog with the same compiler used for your application.
