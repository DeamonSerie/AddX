# AddX Assembly Module

Low-level assembly programming for AddX VMs.

## Registers

### General Purpose (32-bit)

| Name | ID | Description |
|------|-----|-------------|
| REG_RAX | 0 | Return value / accumulator |
| REG_RCX | 1 | Counter / loop |
| REG_RDX | 2 | Data / dividend |
| REG_RBX | 3 | Base |
| REG_RSP | 4 | Stack pointer |
| REG_RBP | 5 | Base pointer |
| REG_RSI | 6 | Source index |
| REG_RDI | 7 | Destination index |

### Extended (64-bit)

| Name | ID |
|------|-----|
| REG_R8 | 8 |
| REG_R9 | 9 |
| REG_R10 | 10 |
| REG_R11 | 11 |
| REG_R12 | 12 |
| REG_R13 | 13 |
| REG_R14 | 14 |
| REG_R15 | 15 |

### Special

| Name | ID | Description |
|------|-----|-------------|
| REG_PC | 255 | Program counter |
| REG_ZF | 256 | Zero flag |
| REG_SF | 257 | Sign flag |
| REG_OF | 258 | Overflow flag |
| REG_CF | 259 | Carry flag |

## Data Transfer Instructions

### MOV
```
MOV(REG_RAX, REG_RBX)  # RAX = RBX
MOV(REG_RCX, 10)       # RCX = 10
```

### MOVI - Move Immediate
```
MOVI(REG_RAX, 100)     # RAX = 100
```

### MOVL - Move Label Address
```
MOVL(REG_RCX, myLabel) # RCX = address of myLabel
```

### POP/PUSH
```
PUSH(REG_RAX)          # Push RAX to stack
POP(REG_RBX)           # Pop top of stack to RBX
```

### LEA - Load Effective Address
```
LEA(REG_RAX, [EBX])    # RAX = address of memory at EBX
```

## Arithmetic Instructions

### ADD/ADDI
```
ADD(REG_RAX, REG_RBX)  # RAX = RAX + RBX
ADDI(REG_RCX, 5)       # RCX = RCX + 5
```

### SUB/SUBI
```
SUB(REG_RAX, REG_RBX)  # RAX = RAX - RBX
SUBI(REG_RCX, 1)       # RCX = RCX - 1
```

### MUL/MULI
```
MUL(REG_RAX, REG_RBX)  # RAX = RAX * RBX
MULI(REG_RCX, 2)       # RCX = RCX * 2
```

### DIV/DIVI
```
DIV(REG_RAX, REG_RBX)  # RAX = RAX / RBX
DIVI(REG_RCX, 3)       # RCX = RCX / 3
```

### MOD/MODI
```
MOD(REG_RAX, REG_RBX)  # RAX = RAX % RBX
MODI(REG_RCX, 10)      # RCX = RCX % 10
```

### INC/DEC
```
INC(REG_RAX)           # RAX = RAX + 1
DEC(REG_RCX)           # RCX = RCX - 1
```

## Logical Instructions

### AND/ANDI
```
AND(REG_RAX, REG_RBX)  # RAX = RAX & RBX
ANDI(REG_RCX, 0xFF)    # RCX = RCX & 255
```

### OR/ORI
```
OR(REG_RAX, REG_RBX)   # RAX = RAX | RBX
ORI(REG_RCX, 0xF0)      # RCX = RCX | 240
```

### XOR/XORI
```
XOR(REG_RAX, REG_RBX)  # RAX = RAX ^ RBX
XORI(REG_RCX, 0xFF)    # RCX = RCX ^ 255
```

### NOT
```
NOT(REG_RAX)           # RAX = ~RAX
```

## Shift Instructions

### SHL/SHR - Bit Shift
```
SHL(REG_RAX, 2)        # RAX = RAX << 2
SHR(REG_RCX, 3)        # RCX = RCX >> 3
```

### SAR - Arithmetic Shift
```
SAR(REG_RAX, 1)        # RAX = RAX >> 1 (sign-extend)
```

### ROL/ROR - Rotate
```
ROL(REG_RAX, 4)        # Rotate left 4 bits
ROR(REG_RCX, 2)        # Rotate right 2 bits
```

## Comparison Instructions

### CMP
```
CMP(REG_RAX, REG_RBX)  # Compare RAX and RBX, set flags
CMP(REG_RCX, 10)       # Compare RCX and 10
```

### TEST
```
TEST(REG_RAX, REG_RBX) # Test bits, set ZF
TEST(REG_RCX, 1)        # Test if bit 0 is set
```

## Branch Instructions

### Unconditional
```
JMP(label)              # Jump to label
JMP(0x1000)            # Jump to address
```

### Conditional (based on flags)
```
JE(label)              # Jump if equal (ZF=1)
JNE(label)             # Jump if not equal (ZF=0)
JG(label)              # Jump if greater (ZF=0, SF=OF)
JGE(label)             # Jump if greater or equal
JL(label)              # Jump if less (SF!=OF)
JLE(label)             # Jump if less or equal
JA(label)              # Jump if above (CF=0, ZF=0)
JB(label)              # Jump if below (CF=1)
```

## Function Instructions

### CALL
```
CALL(myFunction)       # Push PC, jump to function
```

### RET
```
RET                    # Pop PC, return to caller
```

## Example: Add Two Numbers

```
def add(a: int, b: int) -> int
    MOVI(REG_RAX, a)
    MOV(REG_RCX, REG_RAX)
    MOVI(REG_RAX, b)
    ADD(REG_RAX, REG_RCX)
    return REG_RAX
```

## Example: Loop

```
def main()
    MOVI(REG_RCX, 10)    # Counter = 10
    MOVI(REG_RAX, 0)    # Sum = 0
    
loop:
    ADD(REG_RAX, REG_RCX)
    DEC(REG_RCX)
    CMP(REG_RCX, 0)
    JNE(label)
    
    return REG_RAX
```

## Example: Factorial

```
def factorial(n: int) -> int
    CMP(n, 1)
    JE(base_case)
    
    DEC(n)
    CALL(factorial)
    MUL(REG_RAX, n)
    RET
    
base_case:
    MOVI(REG_RAX, 1)
    RET
```

## Complete Example Program

```
# AddX Assembly Program
# Calculates factorial of 5

ORG 0x1000

start:
    MOVI(REG_RAX, 5)
    CALL(factorial)
    HLT

factorial:
    CMP(REG_RAX, 1)
    JE(return_one)
    PUSH(REG_RAX)
    DEC(REG_RAX)
    CALL(factorial)
    POP(REG_RCX)
    MUL(REG_RAX, REG_RCX)
    RET

return_one:
    MOVI(REG_RAX, 1)
    RET
```