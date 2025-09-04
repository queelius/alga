---
name: Bug report
about: Create a report to help us improve the library
title: '[BUG] '
labels: bug
assignees: ''

---

**Describe the bug**
A clear and concise description of what the bug is.

**Environment**
- OS: [e.g., Ubuntu 20.04, macOS Big Sur, Windows 10]
- Compiler: [e.g., GCC 9.3, Clang 12, MSVC 2019]
- C++ Standard: [e.g., C++17, C++20]
- CMake Version: [e.g., 3.16.3]
- Library Version: [e.g., v1.0.0]

**To Reproduce**
Steps to reproduce the behavior:
1. Create a minimal example showing the issue
2. Compile with specific flags
3. Run the program
4. See error

**Minimal Example**
```cpp
#include "parsers/lc_alpha.hpp"

int main() {
    // Your minimal reproduction case here
    auto word = make_lc_alpha("test");
    // ... demonstrate the bug
    return 0;
}
```

**Expected behavior**
A clear and concise description of what you expected to happen.

**Actual behavior**
A clear and concise description of what actually happened.

**Error output**
If applicable, add compiler errors, runtime errors, or unexpected output:
```
Paste error messages here
```

**Mathematical Context**
If the bug relates to algebraic properties:
- Which algebraic law is being violated?
- What mathematical invariant is broken?
- Does this affect type safety or composability?

**Additional context**
Add any other context about the problem here, such as:
- Does it happen with specific input patterns?
- Is it related to performance or correctness?
- Any workarounds you've discovered?

**Checklist**
- [ ] I have searched existing issues to avoid duplicates
- [ ] I have provided a minimal reproduction case
- [ ] I have included all relevant environment details
- [ ] I have tested with the latest version