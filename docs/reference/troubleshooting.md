# Troubleshooting

This document covers common issues encountered when building, installing, or using StreamLog. The issues are organized by when they occur: build-time errors, runtime problems, and platform-specific quirks. Each issue includes the symptom (what you observe), possible causes, and step-by-step solutions. If your issue isn't covered here, the debugging tips section provides techniques for diagnosing less common problems.

## Build Issues

Build-time errors typically relate to missing dependencies, incorrect paths, or linker configuration. These errors occur during compilation or linking and prevent the creation of the final executable. Most build issues can be resolved by ensuring the library is properly installed and that include and library paths are correctly specified. The following subsections address specific error messages you might encounter.

### "cannot find -llog"

**Symptom:**

```
/usr/bin/ld: cannot find -llog
```

**Causes and Solutions:**

1. **Library not installed:**

   ```bash
   sudo make install
   ```

2. **Library in non-standard location:**

   ```bash
   g++ -L/path/to/streamlog/build/lib myapp.cpp -llog -o myapp
   ```

3. **LD_LIBRARY_PATH not set:**
   ```bash
   export LD_LIBRARY_PATH=/path/to/streamlog/build/lib:$LD_LIBRARY_PATH
   ```

---

### "streamlog.hpp: No such file or directory"

**Symptom:**

```
fatal error: streamlog.hpp: No such file or directory
```

**Solutions:**

1. **Add include path:**

   ```bash
   g++ -I/path/to/streamlog/src myapp.cpp -llog
   ```

2. **Install headers:**
   ```bash
   sudo make install
   # Headers go to /usr/local/include/
   ```

---

### "undefined reference to `log(LogLevel)'"

**Symptom:**

```
undefined reference to `log(LogLevel)'
```

**Causes:**

1. **Not linking the library:**

   ```bash
   # Wrong:
   g++ myapp.cpp -o myapp

   # Correct:
   g++ myapp.cpp -llog -o myapp
   ```

2. **Wrong link order:**

   ```bash
   # Wrong (library before source):
   g++ -llog myapp.cpp -o myapp

   # Correct (source before library):
   g++ myapp.cpp -llog -o myapp
   ```

---

### "error while loading shared libraries"

**Symptom:**

```
error while loading shared libraries: streamlog.so: cannot open shared object file
```

**Solutions:**

1. **Set library path:**

   ```bash
   export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
   ```

2. **Update ldconfig:**

   ```bash
   sudo ldconfig
   ```

3. **Use static library instead:**
   ```bash
   g++ myapp.cpp /path/to/streamlog.a -o myapp
   ```

---

## Runtime Issues

Runtime issues occur when the library builds and links successfully but doesn't behave as expected during execution. These problems are often more subtle than build errors because they don't produce obvious error messages. Symptoms include missing log output, garbled text, unexpected colors, or performance degradation. The following subsections cover the most common runtime issues and their solutions.

### No Log Output

**Possible Causes:**

1. **DEBUG_LEVEL too high:**

   Check what level your library was compiled with:

   ```bash
   make DEBUG_LEVEL=1  # Recompile with all levels
   ```

2. **Console output disabled:**

   The singleton was initialized without console output:

   ```cpp
   // First call determines settings
   StreamLog::instance("log.txt", false);  // No console

   // To enable console:
   StreamLog::instance("log.txt", true);   // With console
   ```

3. **Log file not writable:**

   Check permissions on the log file path.

---

### Colors Not Displaying

**Symptom:** Log output shows escape codes like `[1;32m` instead of colors.

**Causes and Solutions:**

1. **Terminal doesn't support ANSI:**
   - Use a modern terminal (Windows Terminal, iTerm2, GNOME Terminal)
   - On Windows, use PowerShell or Windows Terminal instead of cmd.exe

2. **Output redirected:**
   - Colors are stripped when redirecting to file (by design)
   - Piping to another program may lose colors

3. **Running in IDE:**
   - Some IDEs don't render ANSI codes
   - Check IDE terminal settings or run from system terminal

---

### Log File Not Created

**Possible Causes:**

1. **Directory doesn't exist:**

   StreamLog creates directories automatically, but check for permission issues:

   ```bash
   mkdir -p /path/to/log/directory
   ```

2. **No write permission:**

   ```bash
   ls -la /path/to/log/
   # Check you have write access
   ```

3. **Disk full:**

   ```bash
   df -h
   ```

4. **Invalid path characters:**

   Avoid special characters in log file paths.

---

### Garbled Output

**Symptom:** Log messages are interleaved or corrupted.

**Cause:** Multi-threaded access without synchronization.

**Solution:** Add mutex protection:

```cpp
#include <mutex>

std::mutex log_mutex;

void safe_log(LogLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(log_mutex);
    log(level) << msg;
}
```

---

### High Memory Usage

**Possible Causes:**

1. **Logging in tight loop:**

   Each log statement creates temporary objects. In hot loops, consider:

   ```cpp
   // Check level before expensive string operations
   if (DEBUG_LEVEL <= 1) {  // TRACE level
       log(TRACE) << expensiveToString(data);
   }
   ```

2. **Large container logging:**

   Logging large vectors/maps creates large strings:

   ```cpp
   // Instead of logging entire container:
   log(DEBUG) << "Container size: " << container.size();
   ```

---

## Platform-Specific Issues

Some issues are specific to particular operating systems or hardware platforms. macOS handles dynamic libraries differently than Linux, requiring different environment variables and tools. Embedded platforms like NVIDIA Jetson may require cross-compilation toolchains that introduce their own challenges. This section covers issues unique to each supported platform.

### macOS: "dylib not loaded"

**Symptom:**

```
dyld: Library not loaded: streamlog.dylib
```

**Solutions:**

1. **Set DYLD_LIBRARY_PATH:**

   ```bash
   export DYLD_LIBRARY_PATH=/usr/local/lib:$DYLD_LIBRARY_PATH
   ```

2. **Use install_name_tool:**
   ```bash
   install_name_tool -add_rpath /usr/local/lib myapp
   ```

---

### ARM64: Cross-Compilation Issues

**Symptom:** Binary won't run on target device.

**Solutions:**

1. **Use correct toolchain:**

   ```bash
   make aarch64=1
   ```

2. **Match library versions:**

   Ensure target device has compatible C++ runtime.

3. **Verify architecture:**
   ```bash
   file build/lib/streamlog.so
   # Should show: ELF 64-bit LSB shared object, ARM aarch64
   ```

---

## Debugging Tips

When standard troubleshooting doesn't resolve an issue, these debugging techniques can help identify the root cause. Debug builds with symbols enable meaningful stack traces in debuggers and crash reports. Address sanitizers can detect memory corruption that might cause intermittent failures. Simple diagnostic code can verify which log levels are active in a given build.

### Enable Debug Symbols

```bash
make EXTRA_FLAGS="-g -O0 --std=c++11"
```

### Use Address Sanitizer

```bash
make EXTRA_FLAGS="-g -fsanitize=address --std=c++11"
```

### Check Compiled Log Level

Add a startup message:

```cpp
int main() {
    log(TRACE) << "TRACE enabled";
    log(DEBUG) << "DEBUG enabled";
    log(INFO)  << "INFO enabled";
    log(WARN)  << "WARN enabled";
    log(ERROR) << "ERROR enabled";
    log(FATAL) << "FATAL enabled";
    // Only levels >= DEBUG_LEVEL will appear
}
```

### Verify Library Version

Check which library is being loaded:

```bash
ldd myapp | grep streamlog
```
