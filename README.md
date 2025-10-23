<div align="center">

# Anvil - Minimal C Build System

<img src="https://res.cloudinary.com/sohambasak/image/upload/v1760796510/ugn1c4ief9wq1dkh4bm4.png" alt="Anvil Logo" width="300"/>

![Status](https://img.shields.io/badge/Status-Beta-orange)
![License](https://img.shields.io/badge/License-MIT-blue)
![C](https://img.shields.io/badge/C-99-blue)
![Platform](https://img.shields.io/badge/Platform-Linux-green)

A simple, CMake-like build system for C projects written in C. Features include live file watching with auto-rebuild (nodemon-style), wildcard source matching, custom include/output directories, out-of-source builds, and built-in auto-updater. Generates readable Makefiles with zero dependencies.

> **⚠️ IMPORTANT:** This project is in early stage development and may contain bugs. Use with caution in production environments.

</div>

## 📦 Installation

### Debian/Ubuntu
```bash
wget https://github.com/dexter-xd/anvil/releases/download/beta_1.2.0/anvil_1.2.0_amd64.deb
sudo dpkg -i anvil_1.2.0_amd64.deb
```

### From Source
```bash
git clone https://github.com/dexter-xd/anvil.git
cd anvil && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCOPY_TO_EXAMPLE=OFF ..

make && sudo make install
```

## Build Configuration

When building Anvil from source, you can control the build behavior:

```bash
# Default behavior - copies anvil binary to example/ for testing
cmake -DCMAKE_BUILD_TYPE=Release ..

# Production build - no copy to example folder
cmake -DCMAKE_BUILD_TYPE=Release -DCOPY_TO_EXAMPLE=OFF ..
```

**Options:**
- `COPY_TO_EXAMPLE=ON` (default) - Copies built binary to `example/anvil` for testing
- `COPY_TO_EXAMPLE=OFF` - Clean build without example copy

## 🚀 Quick Start

### Create New Project
```bash
# Create a new project with template generator
anvil -c

# Follow the interactive prompts to set up your project structure
```

### Manual Setup
```bash
# Check installation
anvil -v

# Update to latest version (if needed)
anvil -u

# Create build.conf
cat > build.conf << EOF
project = MyApp
version = 1.0.0
target = myapp
sources = src/*
includes = include
cflags = -Wall -O2
EOF

# Generate and build
anvil build.conf
cd build && make run
```

## 🖥️ Usage

```bash
anvil [-v|-w|-wr|-u|-c] <buildfile>

  -v          Show version
  -w          Watch mode (auto-rebuild) (supported for both multiple targets and single target)
  -wr         Watch & run mode (only for single target)
  -u          Update to latest version
  -u <ver>    Update to specific version (e.g., anvil -u 1.1.0)
  -c          Create new project template (interactive)
```

## ⚙️ Configuration

### Single Target (Legacy)
| Option | Description | Example |
|--------|-------------|---------|
| `project` | Project name | `project = MyApp` |
| `version` | Project version | `version = 1.2.0` |
| `target` | Executable name | `target = myapp` |
| `sources` | Source files | `sources = src/* main.c` |
| `includes` | Include dirs | `includes = include` |
| `cflags` | Compiler flags | `cflags = -Wall -O2` |
| `ldflags` | Linker flags | `ldflags = -lm` |
| `output_dir` | Output directory | `output_dir = bin` |

### Multi-Target Configuration
```conf
project = WebApp
version = 1.5.2
includes = include
cflags = -Wall -O2
output_dir = bin

[target:server]
sources = src/server.c src/common.c
ldflags = -lpthread
[/target]

[target:client]
sources = src/client.c src/common.c
ldflags = -lncurses
[/target]
```

**Multi-target commands:**
- `make all` - Build all targets
- `make server` - Build specific target
- `make run-server` - Run specific target
- `make clean` - Clean all targets

## 🏷️ Version Support

Anvil automatically defines a `VERSION` macro from your build.conf version field:

```c
#include <stdio.h>

int main() {
    printf("MyApp v%s\n", VERSION);  // Prints: MyApp v1.0.0
    return 0;
}
```

The version flows from `build.conf` → `Makefile` → compiled binary, similar to CMake's version handling.

## 💡 Development Workflow

```bash
# Start watch (supported for both multiple targets and single target)
anvil -w build.conf

# Start watch & run mode (for single target)
anvil -wr build.conf

# Edit code, save, see results instantly!
```

## 🎯 Project Template Generator

Anvil includes an interactive project template generator to quickly scaffold new C projects:

```bash
anvil -c
```

The template generator will prompt you for:
- **Project name**: Enter a custom name or use '.' for current directory
- **Version**: Project version (default: 1.0.0)
- **Source structure**: Whether to create `src/` folder
- **Include structure**: Whether to create `include/` folder  
- **Multi-target**: Whether the project needs multiple executables

Features:
- Creates appropriate directory structure
- Generates hello world program with VERSION macro
- Creates properly configured `build.conf`
- Supports both single and multi-target configurations
- Smart project naming (uses "project" as default if no name provided)

## 🔄 Auto-Update Feature

Anvil includes a built-in updater that can download and install new versions automatically:

```bash
# Update to latest version
anvil -u

# Update to specific version
anvil -u 1.2.0

# Update to beta version
anvil -u beta_1.2.0
```

The updater:
- Fetches releases from GitHub
- Downloads the appropriate .deb package
- Installs using `sudo dpkg -i`
- Works with both stable and beta releases

## 📁 Example Structure

### Single Target
```
my-project/
├── build.conf
├── src/
│   ├── main.c
│   └── utils.c
├── include/
│   └── utils.h
└── build/          # Generated
    ├── myapp
    └── Makefile
```

### Multi-Target
```
web-project/
├── multi-target.conf
├── src/
│   ├── server.c
│   ├── client.c
│   └── common.c
├── include/
│   └── common.h
└── build/          # Generated
    ├── bin/
    │   ├── server
    │   └── client
    └── Makefile
```

## 🗑️ Uninstall

```bash
sudo apt remove anvil  # or sudo rm /usr/bin/anvil
```

## 📄 License

MIT License - see LICENSE file.

---
