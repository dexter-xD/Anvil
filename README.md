<div align="center">

# Anvil - Minimal C Build System

<img src="https://res.cloudinary.com/sohambasak/image/upload/v1760796510/ugn1c4ief9wq1dkh4bm4.png" alt="Anvil Logo" width="300"/>

![Status](https://img.shields.io/badge/Status-Early%20Stage-red)
![License](https://img.shields.io/badge/License-MIT-orange)
![C](https://img.shields.io/badge/C-99-blue)
![Platform](https://img.shields.io/badge/Platform-Linux-green)
![Build](https://img.shields.io/badge/Build-CMake-red)

A simple, CMake-like build system for C projects written in C. It provides a minimalistic project structure with pre-configured build generation, Makefile creation, and a few useful configuration options. I share it here in case it might be useful to others.

> **⚠️ IMPORTANT:** This project is in early stage development and may contain bugs. Use with caution in production environments.

</div>

## ⚡ Features

- **Minimalistic project structure** - Clean and organized codebase
- **Wildcard source file matching** - Support for `src/*` patterns
- **Multiple include directories** - Flexible header organization
- **Custom output directories** - Control where executables are placed
- **Out-of-source builds** - Everything organized in `build/` directory
- **Automatic dependency tracking** - Smart rebuild system
- **Built-in targets** - `run` and `clean` targets included
- **CMake integration** - Modern build system with optional features
- **File watching** - Auto-rebuild on source/header file changes (`-w` flag)
- **Watch & Run mode** - Auto-rebuild and run on changes (`-wr` flag)
- **Smart monitoring** - Watches both source files and header files in include directories

## 🚀 Quick Start

### 1. Build Anvil

```bash
mkdir build && cd build
cmake .. && make
```

### 2. Try the Example

```bash
cd example
./anvil build.conf          # Generate Makefile
cd build && make run        # Build and run

# Or use watch mode for development:
./anvil -w build.conf       # Auto-rebuild on file changes
./anvil -wr build.conf      # Auto-rebuild and run on file changes
```

### 3. Create Your Own Project

Create a `build.conf` file:

```properties
project = MyProject
target = myapp
sources = src/*
includes = include
cflags = -Wall -O2 -std=c99
ldflags = -lm
output_dir = bin
```

### 4. Generate and Build

```bash
./anvil build.conf
cd build
make           # compile
make run       # compile and run
make clean     # clean build artifacts
```

## 📁 Project Structure

```
Anvil/
├── CMakeLists.txt          # CMake build configuration
├── include/
│   └── anvil.h            # Header file
├── src/
│   ├── main.c             # Main entry point
│   ├── config_parser.c    # Configuration file parser
│   ├── file_utils.c       # File and directory utilities
│   ├── makefile_generator.c # Makefile generation
│   ├── string_utils.c     # String manipulation utilities
│   └── watch_system.c     # File watching and auto-build system
└── example/               # Example project
    ├── build.conf         # Build configuration
    ├── src/               # Example source files
    └── include/           # Example headers
```

## ⚙️ Configuration Options

| Option | Description | Example |
|--------|-------------|---------|
| `project` | Project name | `project = MyApp` |
| `target` | Executable name | `target = myapp` |
| `sources` | Source files (supports wildcards) | `sources = src/* main.c` |
| `includes` | Include directories | `includes = include headers` |
| `cflags` | Compiler flags | `cflags = -Wall -O2` |
| `ldflags` | Linker flags | `ldflags = -lm -lpthread` |
| `output_dir` | Output directory (optional) | `output_dir = bin` |

## 🏗️ Build Options

### Using CMake Directly

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..                    # Default: copies to example
cmake -DCMAKE_BUILD_TYPE=Release -DCOPY_TO_EXAMPLE=OFF ..  # No copy to example
make -j4
```

### Available Options

- `COPY_TO_EXAMPLE` (ON/OFF, default: ON): Automatically copy the executable to `example/anvil` after successful build
- `CMAKE_BUILD_TYPE` (Debug/Release, default: Release): Build configuration

## 📂 Generated Directory Structure

**Without `output_dir`:**
```
project/
├── build.conf
├── src/
├── include/
└── build/
    ├── myapp       # executable
    ├── obj/        # object files
    └── Makefile
```

**With `output_dir = bin`:**
```
project/
├── build.conf
├── src/
├── include/
└── build/
    ├── bin/
    │   └── myapp   # executable
    ├── obj/
    └── Makefile
```

## � EWatch Mode Features

Anvil includes powerful file watching capabilities for seamless development:

```bash
# Basic usage
./anvil build.conf          # Generate Makefile once

# Watch mode - auto-rebuild on file changes
./anvil -w build.conf       # Watches source and header files

# Watch & Run mode - auto-rebuild and run on changes  
./anvil -wr build.conf      # Perfect for development workflow
```

**What gets watched:**
- All source files (`.c` files)
- All header files (`.h` files) in include directories
- Recursive monitoring of include subdirectories

## 💡 Example Usage

```properties
# build.conf
project = Calculator
target = calc
sources = src/*.c
includes = include
cflags = -Wall -Wextra -O2 -std=c11
ldflags = -lm
output_dir = bin
```

```bash
# Development workflow
./anvil -wr build.conf      # Start watch & run mode
# Edit your code, save, and see instant results!

# Or traditional workflow
./anvil build.conf
cd build && make run
```

## 🔧 Development

The project is organized into modular components:

- **config_parser.c**: Parses build.conf files
- **file_utils.c**: File operations and glob expansion  
- **makefile_generator.c**: Generates Makefiles
- **string_utils.c**: String manipulation utilities
- **watch_system.c**: File watching, auto-build, and run functionality
- **main.c**: Main program logic and command-line interface

## 📄 License

This project is licensed under the MIT License.

---