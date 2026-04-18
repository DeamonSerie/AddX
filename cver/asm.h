#ifndef ADDX_ASM_H
#define ADDX_ASM_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LABEL_LENGTH 128
#define MAX_INSTRUCTIONS 4096
#define MAX_OUTPUT_SIZE 8192

typedef enum {
    TOKEN_LABEL,
    TOKEN_INSTRUCTION,
    TOKEN_REGISTER,
    TOKEN_IMMEDIATE,
    TOKEN_ADDRESS,
    TOKEN_IDENTIFIER,
    TOKEN_DIRECTIVE,
    TOKEN_COMMENT,
    TOKEN_NEWLINE,
    TOKEN_END
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_LINE_LENGTH];
    int line;
    int column;
} Token;

typedef enum {
    OP_NONE,
    OP_REGISTER,
    OP_IMMEDIATE,
    OP_MEMORY,
    OP_LABEL,
    OP_RELATIVE
} OperandType;

typedef struct {
    OperandType type;
    char value[MAX_LINE_LENGTH];
    int immediate;
    int scale;
} Operand;

typedef struct {
    char mnemonic[MAX_LINE_LENGTH];
    Operand operands[4];
    int operandCount;
    int address;
    char label[MAX_LABEL_LENGTH];
} Instruction;

typedef struct {
    char name[MAX_LABEL_LENGTH];
    int address;
    int referenceCount;
    int references[256];
} Label;

typedef struct {
    Instruction instructions[MAX_INSTRUCTIONS];
    int instructionCount;
    
    Label labels[MAX_INSTRUCTIONS];
    int labelCount;
    
    char output[MAX_OUTPUT_SIZE];
    uint8_t machineCode[MAX_OUTPUT_SIZE];
    int machineCodeSize;
    
    int currentAddress;
    int entryPoint;
    bool verbose;
} Assembler;

void Assembler_Init(Assembler* a);
void Assembler_Destroy(Assembler* a);
bool Assembler_Assemble(Assembler* a, const char* inputFile, const char* outputFile);
void Assembler_SetVerbose(Assembler* a, bool verbose);

int asm_main(int argc, char** argv);

#endif