# Installation Guide

This document provides comprehensive instructions for building and installing the Algebraic Parsers library.

## System Requirements

### Minimum Requirements
- **C++ Compiler**: GCC 7+, Clang 5+, or MSVC 2017+
- **C++ Standard**: C++17 or later (C++20 recommended)
- **CMake**: Version 3.14 or later
- **RAM**: 2GB minimum for compilation
- **Disk Space**: ~100MB for full build including tests

### Recommended Development Environment
- **C++ Compiler**: GCC 9+ or Clang 10+
- **CMake**: Version 3.16+
- **Optional Tools**:
  - `clang-format` for code formatting
  - `cppcheck` for static analysis
  - `lcov` and `genhtml` for test coverage reports
  - `doxygen` for API documentation generation

## Quick Install

### Clone and Build
```bash
git clone https://github.com/yourusername/algebraic_parsers.git
cd algebraic_parsers
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run Tests
```bash
make test
```

### Install System-wide (Optional)
```bash
sudo make install
```

## Detailed Build Instructions

### 1. Clone the Repository
```bash
git clone https://github.com/yourusername/algebraic_parsers.git
cd algebraic_parsers
```

### 2. Configure Build Options

#### Standard Build
```bash
mkdir build && cd build
cmake ..
```

#### Debug Build
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

#### Release Build with Optimizations
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

#### Coverage Build
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Coverage ..
```

### 3. Build the Library

#### Build All Components
```bash
make -j$(nproc)
```

#### Build Only Core Library
```bash
make algebraic_parsers porter2stemmer
```

#### Build Specific Tests
```bash
make lc_alpha_test porter2stemmer_test
```

## Platform-Specific Instructions

### Linux (Ubuntu/Debian)

#### Install Dependencies
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libgtest-dev  # For testing
sudo apt install doxygen graphviz  # For documentation
sudo apt install lcov  # For coverage
```

#### Build
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Linux (RHEL/CentOS/Fedora)

#### Install Dependencies
```bash
# RHEL/CentOS
sudo yum install gcc-c++ cmake git make
sudo yum install gtest-devel doxygen graphviz lcov

# Fedora
sudo dnf install gcc-c++ cmake git make
sudo dnf install gtest-devel doxygen graphviz lcov
```

### macOS

#### Install Dependencies
```bash
# Using Homebrew
brew install cmake git
brew install googletest doxygen graphviz lcov
```

#### Build
```bash
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### Windows (Visual Studio)

#### Prerequisites
- Visual Studio 2017 or later with C++ development tools
- CMake 3.14+
- Git for Windows

#### Build
```batch
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

## CMake Configuration Options

### Standard Options
```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DCMAKE_CXX_STANDARD=17
```

### Advanced Options
```bash
cmake .. \
  -DBUILD_TESTING=ON \
  -DBUILD_EXAMPLES=ON \
  -DENABLE_COVERAGE=ON \
  -DENABLE_STATIC_ANALYSIS=ON
```

## Testing

### Run All Tests
```bash
make test
```

### Run Specific Test Categories
```bash
./lc_alpha_test
./porter2stemmer_test
```

### Generate Coverage Report
```bash
# Only available in Coverage build
make coverage
# Open coverage_html/index.html in browser
```

## Installation

### System Installation
```bash
sudo make install
```

Default installation paths:
- Headers: `/usr/local/include/parsers/`
- Libraries: `/usr/local/lib/`
- CMake configs: `/usr/local/lib/cmake/algebraic_parsers/`

### Custom Installation Path
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
make install
```

### Using in Your Project

#### With CMake
```cmake
find_package(algebraic_parsers REQUIRED)
target_link_libraries(your_target algebraic_parsers::algebraic_parsers)
```

#### Manual Include
```cpp
#include "parsers/lc_alpha.hpp"
#include "parsers/porter2stemmer.hpp"
```

## Development Tools

### Code Formatting
```bash
make format  # Requires clang-format
```

### Static Analysis
```bash
make static_analysis  # Requires cppcheck
```

### Documentation Generation
```bash
make docs  # Requires doxygen
```

## Troubleshooting

### Common Build Issues

#### GCC Version Too Old
**Error**: `error: #error This file requires compiler and library support for the ISO C++ 2017 standard`

**Solution**: Install GCC 7+ or Clang 5+
```bash
# Ubuntu
sudo apt install gcc-9 g++-9
export CC=gcc-9
export CXX=g++-9
```

#### CMake Too Old
**Error**: `CMake 3.14 or higher is required`

**Solution**: Install newer CMake
```bash
# Remove old version
sudo apt remove cmake
# Install from official source
wget https://cmake.org/files/v3.20/cmake-3.20.0-Linux-x86_64.sh
sudo sh cmake-3.20.0-Linux-x86_64.sh --prefix=/usr/local --skip-license
```

#### Missing Dependencies
**Error**: Various missing header errors

**Solution**: Install development packages
```bash
# Ubuntu/Debian
sudo apt install build-essential libstdc++-dev

# RHEL/CentOS
sudo yum groupinstall "Development Tools"
```

### Performance Issues

#### Slow Compilation
- Use `-j$(nproc)` for parallel builds
- Ensure sufficient RAM (2GB+ recommended)
- Consider using `ccache` for repeated builds

#### Large Binary Size
- Use Release build: `-DCMAKE_BUILD_TYPE=Release`
- Enable link-time optimization: `-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`

## Advanced Configuration

### Custom Compiler Flags
```bash
cmake .. \
  -DCMAKE_CXX_FLAGS="-march=native -O3" \
  -DCMAKE_BUILD_TYPE=Release
```

### Cross-Compilation
```bash
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=path/to/toolchain.cmake \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=arm64
```

## Getting Help

- **Issues**: Report bugs on GitHub Issues
- **Discussions**: Use GitHub Discussions for questions
- **Documentation**: See README.md and other .md files
- **Examples**: Check the `examples/` directory

---

For the latest installation instructions, please check the repository's README.md file.