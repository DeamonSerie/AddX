// AddX Assembly Assembler
// Translates AddX assembly to machine code or bytecode

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LABEL_LENGTH 128
#define MAX_INSTRUCTIONS 4096

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
    
    char labelNames[MAX_INSTRUCTIONS][MAX_LABEL_LENGTH];
    int labelAddresses[MAX_INSTRUCTIONS];
    int labelCount2;
    
    char output[8192];
    uint8_t machineCode[8192];
    int machineCodeSize;
    
    int currentAddress;
    int entryPoint;
    bool verbose;
} Assembler;

void initAssembler(Assembler* a) {
    memset(a, 0, sizeof(Assembler));
}

char* readFile(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(size + 1);
    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}

char* trim(char* s) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    
    char* end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

int countOperands(const char* line) {
    int count = 0;
    bool inOperand = false;
    
    for (int i = 0; line[i]; i++) {
        if (line[i] == ',' || line[i] == '\t' || line[i] == ' ') {
            if (inOperand) {
                count++;
                inOperand = false;
            }
        } else {
            inOperand = true;
        }
    }
    
    if (inOperand) count++;
    return count;
}

int getInstructionSize(const char* line) {
    char mnemonic[MAX_LINE_LENGTH];
    sscanf(line, "%s", mnemonic);
    return 4;
}

void firstPass(Assembler* a, const char* source) {
    char line[MAX_LINE_LENGTH];
    char* copy = strdup(source);
    char* tok = strtok(copy, "\n");
    int address = 0;
    int lineNum = 0;
    
    while (tok) {
        lineNum++;
        
        char* comment = strchr(tok, ';');
        if (comment) *comment = '\0';
        
        char* trimmed = trim(tok);
        
        if (strlen(trimmed) == 0) {
            tok = strtok(NULL, "\n");
            continue;
        }
        
        if (strchr(trimmed, ':')) {
            char* colon = strchr(trimmed, ':');
            *colon = '\0';
            char* labelName = trim(trimmed);
            
            strcpy(a->labels[a->labelCount].name, labelName);
            a->labels[a->labelCount].address = address;
            a->labelCount++;
            
            trimmed = trim(colon + 1);
            if (strlen(trimmed) == 0) {
                tok = strtok(NULL, "\n");
                continue;
            }
        }
        
        if (trimmed[0] == '.') {
            if (strstr(trimmed, ".byte")) {
                address += countOperands(trimmed);
            } else if (strstr(trimmed, ".word")) {
                address += countOperands(trimmed) * 4;
            } else if (strstr(trimmed, ".space")) {
                address += countOperands(trimmed);
            }
            tok = strtok(NULL, "\n");
            continue;
        }
        
        address += getInstructionSize(trimmed);
        tok = strtok(NULL, "\n");
    }
    
    free(copy);
}

void secondPass(Assembler* a) {
    // Generate machine code stub
    a->machineCodeSize = 0;
}

void writeOutput(Assembler* a, const char* output) {
    FILE* f = fopen(output, "wb");
    if (!f) return;
    
    fwrite(a->machineCode, 1, a->machineCodeSize, f);
    fclose(f);
}

void assemble(Assembler* a, const char* input, const char* output) {
    printf("Assembling %s...\n", input);
    
    char* source = readFile(input);
    if (!source) {
        fprintf(stderr, "Error: Cannot read file %s\n", input);
        return;
    }
    
    firstPass(a, source);
    secondPass(a);
    writeOutput(a, output);
    
    free(source);
    
    printf("Assembly complete!\n");
    printf("Generated %d bytes\n", a->machineCodeSize);
}


int AddXmain(int argc, char** argv) {
    Assembler assembler;
    initAssembler(&assembler);
    
    char* inputFile = NULL;
    char* outputFile = NULL;
    bool showHelp = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            showHelp = true;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            assembler.verbose = true;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (!inputFile) {
            inputFile = argv[i];
        } else {
            outputFile = argv[i];
        }
    }
    
    if (showHelp || !inputFile) {
        printf("AddX Assembly Assembler\n");
        printf("Usage: asm [-o output] [-v] input [output]\n");
        printf("Options:\n");
        printf("  -o <file>  Set output file\n");
        printf("  -v         Verbose output\n");
        printf("  -h         Show this help\n");
        return showHelp ? 0 : 1;
    }
    
    if (!outputFile) {
        int len = strlen(inputFile);
        outputFile = (char*)malloc(len + 5);
        strcpy(outputFile, inputFile);
        char* dot = strrchr(outputFile, '.');
        if (dot) strcpy(dot, ".bin");
        else strcat(outputFile, ".bin");
    }
    
    assemble(&assembler, inputFile, outputFile);
    
    if (!outputFile || inputFile == outputFile) free(outputFile);
    
    return 0;
}