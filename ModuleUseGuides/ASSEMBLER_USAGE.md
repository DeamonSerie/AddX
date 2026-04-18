# AddX Assembler Module

AddX interface to the assembler for converting assembly code to machine code or bytecode.

## Basic Usage

### Assemble to Binary

```
result = assemble("source.asm", "output.bin")
if result then
    print("Assembly successful!")
```

### Assemble from File

```
success = assembleFile("program.asm", "program.bin")
```

### Assemble with Options

```
success = assembleWithOptions(
    source = "program.asm",
    output = "program.bin",
    verbose = true,
    useLabels = true
)
```

## Validation

### Validate Assembly Code

```
isValid, errors = validateAssembly(source)
if not isValid then
    print("Errors found:")
    for err in errors do
        print(err)
```

## Formatting

### Format Assembly Code

```
formatted = formatAssembly(source)
print(formatted)
```

Converts raw assembly to proper indentation:

```asm
; Comment
label:
    instruction operand1, operand2
```

## Metadata

### Get Version

```
version = getAssemblerVersion()
print(version)  # "AddX Assembler v1.0"
```

### List Supported Opcodes

```
opcodes = listSupportedOpcodes()
for op in opcodes do
    print(op)
```

Available opcodes: NOP, MOV, MOVI, MOVL, MOVH, ADD, ADDI, SUB, SUBI, MUL, MULI, DIV, DIVI, MOD, MODI, AND, ANDI, OR, ORI, XOR, XORI, NOT, SHL, SHR, SAR, ROL, ROR, CMP, CMPI, TEST, TESTI, JE, JNE, JG, JGE, JL, JLE, JA, JB, JMP, CALL, RET, JMPF, CALLF, PUSH, POP, PUSHF, POPF, LOAD, STORE, LOADB, STOREB, LOADW, STOREW, LOADI, STOREI, HLT, INT, IRET, NMI

### List Supported Registers

```
regs = listSupportedRegisters()
print(regs)  # ["R0", "R1", ..., "SP", "BP", "IP", "FLAGS", "X0", ...]
```

Registers: R0-R15 (general), SP (stack pointer), BP (base pointer), IP (instruction pointer), FLAGS, X0-X7 (extended)

## Example Programs

### Hello World

```
program = getHelloWorldProgram()
print(program)
```

Output:
```asm
; AddX Hello World Program

        ORG 0x1000
        
        MOVI R1, hello_str
        MOVI R2, 0
        CALL print_str
        
        HLT
        
hello_str:
        DB "Hello, World!", 0
```

### Add Two Numbers

```
program = getAddProgram()
print(program)
```

Output:
```asm
; Add Two Numbers Program

        ORG 0x1000
        
        MOVI R1, 10
        MOVI R2, 20
        ADD R3, R1, R2
        
        HLT
```

### Factorial

```
program = getFactorialProgram()
print(program)
```

### Fibonacci

```
program = getFibonacciProgram()
print(program)
```

## Full Example

```
def main()
    print("=== AddX Assembler Demo ===")
    
    program = getAddProgram()
    print("Source:")
    print(program)
    
    isValid, errors = validateAssembly(program)
    print("Valid: " + str(isValid))
    
    if isValid then
        result = assemble(program, "add.bin")
        print("Assembled: " + str(result))
    
    return 0
```

## Assembly Syntax

### Labels

```
my_label:
    ; code
```

### Instructions

```
MOV R1, R2
ADD R3, R1, R2
MUL R1, R1, 10
```

### Directives

```
ORG 0x1000    ; Set origin
DB "text", 0  ; Define bytes
DW 1234       ; Define word
RESB 256      ; Reserve bytes
```

### Comments

```
; This is a comment
```

## Error Handling

```
isValid, errors = validateAssembly(source)
if not isValid then
    for err in errors do
        print(err)
```

Common errors:
- Invalid label names
- Unclosed brackets
- Unknown opcodes
- Invalid register names
- Missing operands