<div align="center">

# Anvil - Minimal C Build System

<img src="https://res.cloudinary.com/sohambasak/image/upload/v1760796510/ugn1c4ief9wq1dkh4bm4.png" alt="Anvil Logo" width="300"/>

![Status](https://img.shields.io/badge/Status-Beta-orange)
![License](https://img.shields.io/badge/License-MIT-blue)
![C](https://img.shields.io/badge/C-99-blue)
![Platform](https://img.shields.io/badge/Platform-Linux-green)

A simple, CMake-like build system for C projects written in C. Features include live file watching with auto-rebuild (nodemon-style), wildcard source matching, custom include/output directories, and out-of-source builds. Generates readable Makefiles with zero dependencies.

> **⚠️ IMPORTANT:** This project is in early stage development and may contain bugs. Use with caution in production environments.

</div>

## 📦 Installation

### Debian/Ubuntu
```bash
wget https://github.com/dexter-xd/anvil/releases/download/beta_1.1.0/anvil_1.1.0_amd64.deb
sudo dpkg -i anvil_1.1.0_amd64.deb
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

```bash
# Check installation
anvil -v

# Create build.conf
cat > build.conf << EOF
project = MyApp
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
anvil [-v|-w|-wr] <buildfile>

  -v     Show version
  -w     Watch mode (auto-rebuild) (supported for both multiple targets and single target)
  -wr    Watch & run mode (only for single target)
```

## ⚙️ Configuration

### Single Target (Legacy)
| Option | Description | Example |
|--------|-------------|---------|
| `project` | Project name | `project = MyApp` |
| `target` | Executable name | `target = myapp` |
| `sources` | Source files | `sources = src/* main.c` |
| `includes` | Include dirs | `includes = include` |
| `cflags` | Compiler flags | `cflags = -Wall -O2` |
| `ldflags` | Linker flags | `ldflags = -lm` |
| `output_dir` | Output directory | `output_dir = bin` |

### Multi-Target Configuration
```conf
project = WebApp
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

## 💡 Development Workflow

```bash
# Start watch (supported for both multiple targets and single target)
anvil -w build.conf

# Start watch & run mode (for single target)
anvil -wr build.conf

# Edit code, save, see results instantly!
```

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
