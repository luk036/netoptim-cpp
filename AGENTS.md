# AGENTS.md - Agent Coding Guidelines for netoptim-cpp

This file provides context and coding guidelines for AI agents working in this repository.

## Project Overview

- **Name**: NetOptim (Network Optimization)
- **Type**: Header-only C++ library for network optimization algorithms
- **Language**: C++20
- **Build System**: CMake (ModernCppStarter template)
- **Test Framework**: doctest
- **Version**: 1.2.2

## Directory Structure

```
netoptim-cpp/
├── include/
│   ├── netoptim/          # Public headers (.hpp)
│   └── ThreadPool.h       # Single legacy header
├── test/
│   └── source/           # Test files (.cpp)
├── cmake/                # CMake helper modules
├── .github/workflows/   # CI configurations
├── CMakeLists.txt       # Root build file
└── AGENTS.md           # This file
```

## Build Commands

### Full Build (all targets)

```bash
cmake -S. -B build
cmake --build build
```

### Build and Run Tests

```bash
cmake -S. -B build
cmake --build build
cd build/test
CTEST_OUTPUT_ON_FAILURE=1 ctest

# Or run executable directly:
./build/test/NetOptimTests
```

### Run a Single Test

The test executable supports doctest's test case filtering. Use the `-tc` option:

```bash
./build/test/NetOptimTests -tc="Test Negative Cycle"
```

Or with wildcards:

```bash
./build/test/NetOptimTests -tc="Test *"
```

### Build with Code Coverage

```bash
cmake -S. -B build -DENABLE_TEST_COVERAGE=1
cmake --build build
```

### Build Documentation

```bash
cmake -S. -B build
cmake --build build --target GenerateDocs
# View: open build/documentation/doxygen/html/index.html
```

### Enable Sanitizers

Configure CMake with `-DUSE_SANITIZER=<Address | Memory | Undefined | Thread | Leak | 'Address;Undefined'>`

### Enable Static Analyzers

Configure CMake with `-DUSE_STATIC_ANALYZER=<clang-tidy | iwyu | cppcheck>`

## Code Style Guidelines

### Formatting (clang-format)

Configuration: `.clang-format` (Google-based)

```bash
# Check formatting
cmake --build build --target format

# Apply formatting
cmake --build build --target fix-format
```

Key settings:
- **BasedOnStyle**: Google
- **ColumnLimit**: 100
- **IndentWidth**: 4
- **AccessModifierOffset**: -2
- **BreakBeforeBraces**: Attach
- **NamespaceIndentation**: All

### Naming Conventions

- **Classes**: `CamelCase` (e.g., `NegCycleFinder`)
- **Functions**: `snake_case` (e.g., `find_neg_cycle`)
- **Variables**: `snake_case` (e.g., `dist`, `gra`, `utx`)
- **Member variables**: Leading underscore (e.g., `_pred`, `_digraph`)
- **Constants**: `kCamelCase` (e.g., `kMaxIterations`)
- **Templates**: `CamelCase` (e.g., `DiGraph`, `Num`)

### File Organization

1. License header (if applicable)
2. `#pragma once` or include guard
3. Doxygen `@file` and `@brief` documentation
4. Includes (grouped, alphabetically within groups)
5. Forward declarations
6. Class/function definitions

### Include Order (per `.clang-format`)

1. Main header (对应的 .hpp for .cpp files)
2. C++ standard library (`<...>`)
3. External libraries (`<...>`)
4. Internal headers (`"..."`)

Example:
```cpp
// test/source/test_neg_cycle.cpp
#include <doctest/doctest.h>

#include <array>
#include <unordered_map>
#include <vector>

#include <netoptim/neg_cycle.hpp>
#include <xnetwork/classes/digraphs.hpp>
#include <xnetwork/generators/testcases.hpp>
```

### Type Conventions

- Use `auto` for return types when clear from context
- Prefer `std::optional` over sentinel values
- Use `std::pair` or structured bindings for multiple returns
- Template parameters: descriptive names (`Graph`, `Mapping`, `Fn`)

### Error Handling

- Use `assert()` for invariants and internal consistency checks
- Use exceptions for exceptional conditions (not for flow control)
- Document exceptions in Doxygen with `@throws`

### Comments

- Use Doxygen for public API documentation
- Include `@param`, `@return`, `@tparam` tags
- Keep implementation comments concise
- Use `// TODO:` for noted improvements

### Testing

- Use doctest framework
- TEST_CASE macro for test cases
- CHECK/REQUIRE for assertions
- Comment each test case with purpose

Example:
```cpp
TEST_CASE("Test Negative Cycle") {
    auto weights = std::array<int, 5>{-5, 1, 1, 1, 1};
    auto gra = create_test_case1(weights);
    const auto hasNeg = do_case(gra);
    CHECK(hasNeg);
}
```

## Dependencies

Managed via CPM.cmake:
- **doctest**: Testing framework
- **Format.cmake**: Code formatting tools
- **PackageProject.cmake**: Installable target creation
- **xnetwork**: Graph library dependency

## GitHub Actions CI

Workflows in `.github/workflows/`:
- `macos.yml` - macOS build and test
- `ubuntu.yml` - Ubuntu build and test
- `windows.yml` - Windows build and test
- `install.yml` - Installation verification
- `documentation.yaml` - Doxygen generation

## Important Notes

1. **Header-only library**: No compilation for the main library; tests compile against it
2. **In-source builds prohibited**: Must use out-of-source build directory
3. **Windows-specific**: Uses `/permissive-` for standards conformance
4. **Modern C++20 features**: Coroutines supported via `/await` on MSVC