# AddX Compiler & JIT Runtime

AddX is a programming language with both a virtual machine (VM) interpreter and a Just-In-Time (JIT) compiler targeting x64 Windows. This guide explains how to build, use, and extend the AddX system.

## Features

- **Parsing**: Indentation-based syntax (similar to Python)
- **Compilation**: AST → bytecode
- **Execution Modes**:
  - Stack-based Virtual Machine (VM)
  - Native JIT compilation (Windows x64)
- **Language Support**:
  - Variables, functions, control flow (`if`, `while`, `for`)
  - Basic types: numbers, strings, booleans, `None`
  - Lists, attribute access, function calls
  - Basic object-oriented features (classes, attributes)
  - Pointer operations (`&`, `*`, `sizeof`)
  - Memory management (`new`, `delete`)
  - Standard output (`print`)
  - Type inheritance (`inherit` keyword)

## Building the Project

### Prerequisites

- **Windows** (for JIT component)
- **GCC compiler** (via MSYS2, MinGW-w64, or similar)
- **Code::Blocks** (optional, for IDE builds)

### Command Line Build

```bash
# Compile all source files into an executable
gcc -o AddX.exe main.c fast_parse.c compiler.c vm.c jit.c

# For debugging builds, you may want to add:
gcc -g -o AddX.exe main.c fast_parse.c compiler.c vm.c jit.c
```

### Code::Blocks Build

1. Open `AddX.cbp` in Code::Blocks
2. Select Build Target: `Debug` or `Release`
3. Click Build (or press F9)
4. The executable will be generated in `bin/Debug/AddX.exe` or `bin/Release/AddX.exe`

## Using the Compiler

### Basic Usage

```bash
# Run with VM interpreter (default)
AddX.exe your_program.addx

# Run with JIT compiler
AddX.exe your_program.addx --jit
```

### Command Line Arguments

```
AddX.exe <file.addx> [--jit]
```

- `<file.addx>`: Path to the source file to compile and run
- `--jit`: Optional flag to use the JIT compiler instead of the VM interpreter

### Module Imports

AddX supports importing code from other AddX files using the `import` statement. This allows you to organize code across multiple files and reuse functions and variables.

```addx
import module_name
```

The importer will look for `module_name.addx` in the current directory and include all its contents in your program.

**Example:**
```addx
# math_utils.addx
def add(a, b):
    return a + b

def multiply(a, b):
    return a * b

# main.addx
import math_utils

def main():
    result = math_utils.add(5, 3)
    print(result)
```

**Features:**
- Recursive imports (modules can import other modules)
- Maximum import depth: 5 levels to prevent infinite loops

## Language Reference

### Syntax Overview

AddX uses indentation to define blocks (similar to Python). Statements are separated by newlines.

```addx
# This is a comment
def main():
    x = 42
    if x > 0:
        print("Positive")
    else:
        print("Non-positive")
```

### Data Types

- **Numbers**: `42`, `3.14` (stored as double precision floats)
- **Strings**: `"hello"`, `'world'`
- **Booleans**: `True`, `False`
- **None**: `None` (null value)
- **Lists**: `[1, 2, 3]`

### Variables

Variables are dynamically typed but declarations can include type annotations:

```addx
x: int = 10          # Explicit type declaration
y = "hello"          # Type inferred
```

### Functions

```addx
def factorial(n: int) -> int:
    if n <= 1:
        return 1
    return n * factorial(n - 1)

def main():
    print(factorial(5))  # Outputs: 120
```

### Control Flow

```addx
# If statement
if condition:
    # then branch
else:
    # else branch

# While loop
while condition:
    # loop body

# For loop (with range)
for i in range(10):
    print(i)

# For loop (with explicit start/end/step)
for i in range(0, 20, 2):
    print(i)  # 0, 2, 4, ..., 18
```

### Lists

```addx
numbers = [1, 2, 3, 4, 5]
numbers[0] = 10          # List element assignment
first = numbers[0]       # List element access
length = len(numbers)    # Using sizeof on list variable
```

### Pointer Operations (Advanced)

```addx
x: int = 42
ptr: int* = &x           # Address-of operator
value: int = *ptr        # Dereference operator
size: int = sizeof(int)  # Size of type
```

### Classes

```addx
class Point:
    x: float
    y: float
    
    def init(self, x_val: float, y_val: float):
        self.x = x_val
        self.y = y_val
    
    def distance(self) -> float:
        return (self.x * self.x + self.y * self.y) ** 0.5

def main():
    p = Point()
    p.init(3.0, 4.0)
    print(p.distance())  # Outputs: 5.0
```

### Inherit Keyword

AddX supports inheriting traits from basic types using the `inherit` keyword. This allows classes and functions to automatically gain methods based on the inherited type.

#### Class Inheritance
Classes can inherit from basic types to get trait-specific methods:
```addx
class Calculator inherit int:
    def add_values(a, b):
        return self.add(a, b)  // Uses inherited add method
        
    def multiply_values(a, b):
        return self.mul(a, b)  // Uses inherited mul method

class TextProcessor inherit str:
    def combine(a, b):
        return self.add(a, b)  // String concatenation
        
    def equals(a, b):
        return self.eq(a, b)   // String equality

class LogicGate inherit bool:
    def and_gate(a, b):
        return self.and(a, b)  // Boolean AND
        
    def or_gate(a, b):
        return self.or(a, b)   // Boolean OR
```

#### Function Return Types
Functions can specify `{inherit}` as a return type:
```addx
def get_number() -> {inherit}:
    return 42
    
def get_text() -> {inherit}:
    return "hello"

def main():
    num = get_number()  // Returns int
    txt = get_text()    // Returns str
```

#### Standalone Inherit Statements
The `inherit` keyword can be used inside functions to gain type-specific traits:
```addx
def process_value(x):
    inherit int  // Gain integer traits in this scope
    return x * 2
    
def process_text(s):
    inherit str  // Gain string traits in this scope
    return s + " processed"
```

**Available Traits by Type:**
- **int/float**: add, sub, mul, div, mod, eq, ne, lt, gt, le, ge
- **str**: add, eq, ne, lt, gt, le, ge (string operations)
- **bool**: and, or, not, eq, ne (boolean operations)

### Memory Management

```addx
buffer: int* = new int[10]  # Allocate array of 10 integers
# ... use buffer ...
delete buffer               # Free memory
```

## Examples

### Inherit Keyword Demonstration (`inherit_example.addx`)
This example shows the new inherit keyword functionality:

```addx
// Class inheriting from int - gets integer math methods
class MathOp inherit int:
    def add_numbers(a, b):
        return self.add(a, b)
        
    def multiply_numbers(a, b):
        return self.mul(a, b)

// Class inheriting from str - gets string methods
class StringUtils inherit str:
    def combine_strings(a, b):
        return self.add(a, b)
        
    def strings_equal(a, b):
        return self.eq(a, b)

// Function with {inherit} return type
def get_value() -> {inherit}:
    return 42

def main():
    print("=== Inherit Keyword Demo ===")
    
    // Test class inheritance
    math = MathOp()
    strings = StringUtils()
    
    print("Objects created successfully")
    
    // Test function return types
    val = get_value()
    print("Function with {inherit} return:", val)
    
    print("=== Demo Complete ===")
```

### Hello World (`hello.addx`)

```addx
def main():
    print("Hello, World!")
```

Run:
```bash
AddX.exe hello.addx
```

### Fibonacci (`fib.addx`)

```addx
def fib(n: int) -> int:
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)

def main():
    for i in range(10):
        print(fib(i))
```

Run:
```bash
AddX.exe fib.addx --jit
```

## Extending the Language

### Adding New Features

1. **Parser (`fast_parse.c`)**:
    - Add new AST node types in `fast_parse.h`
    - Implement parsing functions for new constructs
    - Update `parse_block` or expression parsers as needed
    - For the `inherit` keyword: added handling in `class_()` and `fun()` methods

2. **Compiler (`compiler.c`)**:
    - Add new OpCode values in `compiler.h` if needed
    - Implement compilation logic in `compile_expression` or `compile_node`
    - Handle any new data types or operations
    - For the `inherit` keyword: enhanced `compile_class()` to automatically add trait methods and added `compile_inherit()` method

3. **VM (`vm.c`)**:
    - Add case statements for new opcodes in the `switch` in `run_vm`
    - Implement the behavior for each new opcode
    - For the `inherit` keyword: added `COMMENT = auto()` to OpCode enum and handler in run loop

4. **JIT (`jit.c`)**:
    - Add emission functions for new opcodes
    - Update the `switch` statement in `jit_compile`
    - Ensure proper register usage and calling conventions

### Testing Changes

After modifications:
1. Rebuild the project
2. Test with existing examples to ensure no regressions
3. Create test cases for new features
4. Verify both VM and JIT modes produce identical results

## Troubleshooting

### Common Issues

- **"undefined reference" errors**: Ensure all `.c` files are included in the build
- **JIT compilation fails**: The JIT component is Windows-specific and requires executable memory allocation
- **Stack overflow in recursion**: The VM has limited stack size; consider iterative solutions for deep recursion
- **Type mismatches**: The language is dynamically typed but the VM expects numeric values for arithmetic operations

### Debugging

- Build with `-g` flag for debug symbols
- Use `printf` statements strategically in the parser/compiler/VMI
- Check generated bytecode by adding debug output in compiler
- For JIT issues, verify memory protection settings work on your Windows version

## License

This programming language is under the Apache 2.0 license and is subject to its terms and use requirements

---

*Happy coding with AddX!*