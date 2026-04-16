# AddX Python Usage Guide

AddX is a programming language that combines Python-like syntax with C++-style type systems. This guide explains how to use the Python implementation of AddX.

## Table of Contents
1. [Quick Start](#quick-start)
2. [Basic Usage with addxrun.py](#basic-usage-with-addxrunpy)
3. [Full Compiler & VM Pipeline](#full-compiler--vm-pipeline)
4. [Direct API Usage](#direct-api-usage)
5. [Skipping Tokenization](#skipping-tokenization)
6. [Language Syntax Overview](#language-syntax-overview)
7. [Examples](#examples)

## Quick Start

To run a simple AddX program:

```bash
python addxrun.py hello.addx
```

This will output: `Hello from AddX!`

## Basic Usage with addxrun.py

The `addxrun.py` script provides the simplest way to run AddX programs. It includes a built-in assembler and VM.

### Usage
```bash
python addxrun.py <file.addx>
```

### How it works
1. Reads the AddX source file
2. Assembles it into bytecode using a simple assembler
3. Executes the bytecode using a simple stack-based VM
4. Outputs any print statements to stdout

### Example
Create a file `hello.addx`:
```addx
def main() -> int {
    print("Hello from AddX!")
    return 0
}
```

Run it:
```bash
python addxrun.py hello.addx
# Output: Hello from AddX!
```

## Full Compiler & VM Pipeline

For more advanced usage, AddX provides a separate compiler and VM that mirror the architecture of the Python implementation.

### Usage
```bash
python compiler.py <file.addx>
```

This performs:
1. Parsing: Source → AST
2. Compilation: AST → Bytecode
3. Execution: Bytecode → VM → Output

### Components
- `compiler.py`: Main compiler interface
- `parser.py`: Parser that converts tokens to AST
- `tokenizer.py`: Tokenizer that converts source to tokens
- `vm.py`: Virtual machine that executes bytecode

## Direct API Usage

You can use the AddX components directly in your Python code for more control.

### Importing Modules
```python
from tokenizer import tokenize
from parser import parse
from compiler import compile_program
from vm import AddXVM
```

### Step-by-Step Processing

```python
# 1. Tokenize source
source = '''def main() -> int {
    print("Hello from AddX!")
    return 0
}'''
tokens = tokenize(source)

# 2. Parse tokens to AST
ast = parse(source, tokens=tokens)  # Skip re-tokenization
# OR: ast = parse(source)          # Let parse handle tokenization

# 3. Compile AST to bytecode
instructions, functions, classes = compile_program(ast)

# 4. Execute bytecode
vm = AddXVM()
vm.functions = functions
vm.classes = classes
result = vm.run(instructions['main'].instructions, {})
print(f"Result: {result}")
```

### Skipping Tokenization

As of the latest update, you can skip the tokenization step when you already have tokens:

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

This is useful when:
- Parsing the same source multiple times
- Integrating with external tokenizers
- Implementing caching mechanisms

## Language Syntax Overview

### Basic Structure
```addx
def function_name(param: type, param2: type2) -> return_type {
    // statements
    return value
}
```

### Variables and Types
```addx
// Explicit typing
x: int = 42
y: float = 3.14
name: str = "AddX"
flag: bool = true
numbers: list[int] = [1, 2, 3, 4, 5]
person: dict[str, str] = {"name": "John", "age": "30"}

// Type inference (when assigned without type)
z = 100           // Inferred as int
pi = 3.14159      // Inferred as float
```

### Control Flow

#### If/Else Statements
```addx
if (condition) {
    // true branch
} elif (another_condition) {
    // else if branch
} else {
    // false branch
}
```

#### Loops
```addx
// While loop
i: int = 0
while (i < 10) {
    print(i)
    i = i + 1
}

// For loop with range
for (i in range(0, 10)) {
    print(i)
}

// For loop over collection
for (item in items) {
    print(item)
}
```

### Functions
```addx
def factorial(n: int) -> int {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

// Usage
result: int = factorial(5)  // result = 120
```

### Print Statement
```addx
print("Hello, World!")           // String literal
print(variable)                  // Variable value
print(42)                        // Integer literal
print(3.14)                      // Float literal
print("Value:", x, "more", y)   // Multiple arguments
```

### Object-Oriented Programming
```addx
class Person {
    name: str
    age: int
    
    def __init__(self(name: str, age: int)) -> void {
        self.name = name
        self.age = age
    }
    
    def introduce() -> void {
        print("Hello, I'm " + self.name + " and I'm " + string(self.age) + " years old")
    }
}

// Usage
p: Person = new Person("Alice", 30)
p.introduce()
delete p  // Manual memory cleanup
```

### Memory Management
AddX provides manual memory management similar to C++:
```addx
// Allocate memory
ptr: int* = new int
*ptr = 42
print(*ptr)  // Prints 42
delete ptr   // Free memory

// Array allocation
arr: int* = new int[5]
arr[0] = 10
arr[1] = 20
// ...
delete[] arr  // Free array memory
```

### Pointers and References
```addx
// Pointers
x: int = 100
ptr: int* = &x      // Address of operator
*ptr = 200          // Dereference operator
print(x)            // Prints 200

// References (similar to C++ references)
y: int& = x         // Reference to x
y = 300             // Modifies x through reference
print(x)            // Prints 300
```

## Examples

See the `.addx` files in the project for complete examples:
- `hello.addx` - Simple "Hello, World!"
- `sample.addx` - More complex example
- `test_*.addx` - Various test cases for language features
- `sample_braces.addx` - Example using brace syntax

### Hello World (hello.addx)
```addx
def main() -> int {
    print("Hello from AddX!")
    return 0
}
```

### Variable Usage (test_simple.addx)
```addx
def main() -> int {
    x: int = 1
    print(x)
    return 0
}
```

### Control Flow (test_control.addx)
```addx
def main() -> int {
    x: int = 5
    if (x > 0) {
        print("Positive")
    } elif (x < 0) {
        print("Negative")
    } else {
        print("Zero")
    }
    return 0
}
```

### Loops (test_while.addx)
```addx
def main() -> int {
    i: int = 0
    while (i < 5) {
        print(i)
        i = i + 1
    }
    return 0
}
```

### Functions (test_class_working.addx)
```addx
def factorial(n: int) -> int {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

def main() -> int {
    print(factorial(5))  // Prints 120
    return 0
}
```

## Best Practices

1. **File Extension**: Use `.addx` for AddX source files
2. **Entry Point**: Programs should have a `main() -> int` function
3. **Return Value**: `main` should return an integer exit code
4. **Statements**: Each statement should be on its own line
5. **Comments**: Use `//` for single-line comments
6. **Indentation**: Use consistent indentation (spaces recommended)

## Troubleshooting

### Common Issues
- **Missing main function**: Ensure your program has a `main() -> int` function
- **Type mismatches**: Make sure variable types match assignments
- **Syntax errors**: Check for missing braces, parentheses, or semicolons
- **Undefined variables**: Ensure all variables are declared before use

### Getting Help
- Examine the `.addx` example files in the project
- Read the `SPEC.md` file for language specification
- Look at the test files (`test_*.addx`) for usage patterns
- Use the debug files (`debug_*.py`) for troubleshooting

## Advanced Usage

### Performance Optimization
When parsing the same source multiple times, tokenize once and reuse tokens:
```python
tokens = tokenize(source)
ast1 = parse(source, tokens=tokens)
ast2 = parse(source, tokens=tokens)  # No re-tokenization overhead
```

### Integration with Other Systems
The modular design allows you to:
- Use the tokenizer independently for lexical analysis
- Use the parser independently for AST generation
- Use the compiler independently for bytecode generation
- Use the VM independently for executing pre-compiled bytecode

## License
[License information would go here]

## Acknowledgments
- Inspired by Python's readability
- Performance considerations from C++
- Community contributions and feedback