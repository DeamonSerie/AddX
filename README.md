# AddX Language

AddX is a programming language that combines Python-like syntax with C++-style type systems. It provides both an interpreted execution via a virtual machine and compilation to C++ for high-performance applications.

## Project Overview

This repository contains implementations of the AddX language in both Python and C:

- **Python Implementation**: The primary development and testing environment
- **C Implementation**: A high-performance alternative that mirrors the Python architecture

## File Structure

```
AddX/
├── # Python Implementation
├── addxrun.py          # Simple interpreter for quick testing
├── compiler.py         # Full compiler to bytecode
├── vm.py              # AddX Virtual Machine
├── parser.py          # Parser with optional tokenization skipping
├── tokenizer.py       # Tokenizer
├── ast_nodes.py       # Abstract Syntax Tree nodes
├── pycpp_parser.py    # Python-like to C++ transpiler
├── *.addx             # AddX source files (tests and examples)
│
├── # C Implementation (in c/ directory)
├── c/
│   ├── avm.h/c        # AddX Virtual Machine (extended opcode set)
│   ├── simple_main.c  # Simple C VM example
│   ├── parser.h/c     # C parser with tokenization skipping
│   ├── compiler.h/c   # C compiler
│   ├── vm.h/c         # C VM (original)
│   ├── tokenizer.h/c  # C tokenizer
│   ├── ast.h/c        # C AST nodes
│   ├── fast_parse.h/c # Fast parser
│   ├── jit.h/c        # JIT compiler
│   └── test_*.c       # C test programs
│
└── # Documentation
    ├── SPEC.md                # Language specification
    ├── CHANGES.md             # Record of modifications
    ├── ADDX_USAGE_GUIDE.md    # Detailed Python usage guide
    └── README.md              # This file
```

## Getting Started

### Prerequisites

- Python 3.6+ (for Python tools)
- C++ compiler (gcc/clang/MSVC) (for C tools and compiled output)

### Running AddX Programs

#### Using the Python Interpreter (addxrun.py)

```bash
python addxrun.py hello.addx
```

#### Using the Full Python Compiler and VM

```bash
python compiler.py hello.addx
```

#### Using the C Implementation

```bash
# Compile the C VM (requires MSVC or gcc)
cd c
cl /nologo /W4 tokenizer.c parser.c ast.c avm.c test_parse_source.c /Fe:test_parse_source.exe
# or with gcc: gcc -o test_parse_source tokenizer.c parser.c ast.c avm.c test_parse_source.c

# Run an AddX program
test_parse_source.exe
```

## Key Features

### Python Version
- Modular design: tokenizer, parser, compiler, VM as separate components
- Optional tokenization skipping for performance
- Full language support: variables, control flow, functions, OOP, memory management, etc.
- Comprehensive test suite

### C Version
- Mirror of Python architecture
- Extended AVM with 45 opcodes supporting full language features
- Manual memory management similar to C++
- Tokenizer and parser with optional tokenization skipping
- Separate VM implementation for performance comparison

## Skipping Tokenization

Both Python and C implementations support skipping the tokenization step when you already have tokens:

**Python:**
```python
from tokenizer import tokenize
from parser import parse

source = '''def main() -> int {
    print("Hello from AddX!")
    return 0
}'''

# Tokenize once
tokens = tokenize(source)

# Parse multiple times without re-tokenizing
ast1 = parse(source, tokens=tokens)
ast2 = parse(source, tokens=tokens)  # No re-tokenization occurs
```

**C:**
```c
#include "parser.h"
#include "tokenizer.h"

const char* source = "def main() -> int {\n    print(\"Hello from AddX!\")\n    return 0\n}";

// Tokenize once
Tokens tokens = tokenize(source);

// Parse multiple times without re-tokenizing
ASTNode* ast1 = parse_source(source, &tokens);
ASTNode* ast2 = parse_source(source, &tokens);  // No re-tokenization occurs

// Remember to free tokens when done
free_tokens(&tokens);
```

## Language Features

- Python-like syntax with optional type annotations
- Manual memory management (new/delete)
- Pointers and references
- Control flow (if/else, for, while)
- Functions with type annotations
- Classes and object-oriented programming
- Data structures (lists, dictionaries)
- Namespaces
- Basic exception handling

## Examples

See the `.addx` files in the project root for complete examples:
- `hello.addx` - Classic "Hello, World!"
- `sample.addx` - More complex example
- `test_*.addx` - Various test cases demonstrating language features

## Building and Testing

### Python Testing
```bash
python test_runner.py hello.addx
```

### C Testing (Windows/MSVC)
```bash
cd c
cl /nologo /W4 tokenizer.c parser.c ast.c avm.c test_parse_source.c /Fe:test_parse_source.exe
test_parse_source.exe
```

## License

[License information would go here]

## Acknowledgments

- Inspired by Python's readability
- Performance considerations from C++
- Community contributions and feedback

--- 

*Last updated: $(Get-Date -Format yyyy-MM-dd)*
