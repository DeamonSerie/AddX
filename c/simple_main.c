#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Opcodes matching the Python addxrun.py
typedef enum {
    OP_LOAD = 0,
    OP_STORE = 1,
    OP_PRINT = 2,
    OP_ADD = 3,
    OP_SUB = 4,
    OP_MUL = 5,
    OP_DIV = 6,
    OP_JMP = 7,
    OP_JZ = 8,
    OP_HLT = 9
} Op;

// Simple instruction structure
typedef struct {
    int op;
    union {
        int int_val;
        char* str_val;
    } arg;
    int is_string; // 1 if arg is string, 0 if integer
} Instruction;

// Simple dynamic array for instructions
typedef struct {
    Instruction* instructions;
    size_t capacity;
    size_t count;
} InstructionArray;

void init_instruction_array(InstructionArray *arr) {
    arr->capacity = 10;
    arr->count = 0;
    arr->instructions = malloc(arr->capacity * sizeof(Instruction));
}

void push_instruction(InstructionArray *arr, int op, int int_val, const char* str_val) {
    if (arr->count >= arr->capacity) {
        arr->capacity *= 2;
        arr->instructions = realloc(arr->instructions, arr->capacity * sizeof(Instruction));
    }
    arr->instructions[arr->count].op = op;
    if (str_val != NULL) {
        arr->instructions[arr->count].arg.str_val = strdup(str_val);
        arr->instructions[arr->count].is_string = 1;
    } else {
        arr->instructions[arr->count].arg.int_val = int_val;
        arr->instructions[arr->count].is_string = 0;
    }
    arr->count++;
}

void free_instruction_array(InstructionArray *arr) {
    for (size_t i = 0; i < arr->count; i++) {
        if (arr->instructions[i].is_string) {
            free(arr->instructions[i].arg.str_val);
        }
    }
    free(arr->instructions);
}

// Helper to trim whitespace
char* trim_whitespace(char* str) {
    char* end;
    
    // Trim leading space
    while (isspace(*str)) str++;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
    
    return str;
}

// Simple assembler (mimics the Python addxrun.py assemble function)
InstructionArray assemble(const char *source) {
    InstructionArray ops;
    init_instruction_array(&ops);
    
    // Make a copy of the source to process line by line
    char *source_copy = strdup(source);
    char *line = strtok(source_copy, "\n");
    
    while (line != NULL) {
        // Trim whitespace
        char *trimmed = trim_whitespace(line);
        
        // Skip empty lines and comments
        if (*trimmed == '\0' || *trimmed == '#') {
            line = strtok(NULL, "\n");
            continue;
        }
        
        // Ignore function definitions
        if (strncmp(trimmed, "def ", 4) == 0) {
            line = strtok(NULL, "\n");
            continue;
        }
        
        // Handle print statements
        if (strncmp(trimmed, "print(", 6) == 0) {
            // Extract the argument inside print()
            char *expr_start = trimmed + 6;
            char *paren = strrchr(expr_start, ')');
            if (paren) *paren = '\0';
            
            // Trim whitespace from the expression
            char *expr = trim_whitespace(expr_start);
            
            if (*expr == '"') {
                // String literal
                expr++; // Skip opening quote
                char *end_quote = strrchr(expr, '"');
                if (end_quote) *end_quote = '\0'; // Remove closing quote
                push_instruction(&ops, OP_PRINT, 0, expr);
            } else if (isdigit(*expr) || (*expr == '-' && isdigit(*(expr+1)))) {
                // Number (including negative)
                int num = atoi(expr);
                push_instruction(&ops, OP_PRINT, num, NULL);
            } else {
                // Identifier (variable name)
                push_instruction(&ops, OP_PRINT, 0, strdup(expr));
            }
        } 
        // Handle simple assignments (var = expr)
        else if (strchr(trimmed, '=') && !strchr(trimmed, ':')) {
            char *equals = strchr(trimmed, '=');
            *equals = '\0';
            
            char *var = trim_whitespace(trimmed);
            char *expr = trim_whitespace(equals + 1);
            
            if (*expr == '"') {
                // String literal
                expr++; // Skip opening quote
                char *end_quote = strrchr(expr, '"');
                if (end_quote) *end_quote = '\0'; // Remove closing quote
                push_instruction(&ops, OP_LOAD, 0, expr);
                push_instruction(&ops, OP_STORE, 0, strdup(var));
            } else if (isdigit(*expr) || (*expr == '-' && isdigit(*(expr+1)))) {
                // Number (including negative)
                int num = atoi(expr);
                push_instruction(&ops, OP_LOAD, num, NULL);
                push_instruction(&ops, OP_STORE, 0, strdup(var));
            } else {
                // Identifier (variable name or another variable)
                push_instruction(&ops, OP_LOAD, 0, strdup(expr));
                push_instruction(&ops, OP_STORE, 0, strdup(var));
            }
        }
        // Ignore if/while/for for now (like Python version)
        else if (strncmp(trimmed, "if ", 3) == 0 ||
                 strncmp(trimmed, "while ", 6) == 0 ||
                 strncmp(trimmed, "for ", 4) == 0) {
            // Do nothing
        }
        
        line = strtok(NULL, "\n");
    }
    
    free(source_copy);
    
    // Add halt instruction
    push_instruction(&ops, OP_HLT, 0, NULL);
    return ops;
}

// Simple VM structures and helper functions
typedef struct {
    char **names;
    int *int_values;
    char **str_values;
    int *is_string;
    size_t capacity;
    size_t count;
} VariableTable;

void init_variable_table(VariableTable *vt) {
    vt->capacity = 16;
    vt->count = 0;
    vt->names = malloc(vt->capacity * sizeof(char*));
    vt->int_values = malloc(vt->capacity * sizeof(int));
    vt->str_values = malloc(vt->capacity * sizeof(char*));
    vt->is_string = malloc(vt->capacity * sizeof(int));
}

void free_variable_table(VariableTable *vt) {
    for (size_t i = 0; i < vt->count; i++) {
        free(vt->names[i]);
        if (vt->is_string[i]) {
            free(vt->str_values[i]);
        }
    }
    free(vt->names);
    free(vt->int_values);
    free(vt->str_values);
    free(vt->is_string);
}

int find_var(VariableTable *vt, const char *name) {
    for (size_t i = 0; i < vt->count; i++) {
        if (strcmp(vt->names[i], name) == 0) {
            return (int)i;
        }
    }
    return -1; // Not found
}

void set_var(VariableTable *vt, const char *name, int int_val, const char *str_val, int is_string) {
    int index = find_var(vt, name);
    if (index >= 0) {
        // Update existing variable
        if (vt->is_string[index]) {
            free(vt->str_values[index]);
        }
        vt->is_string[index] = is_string;
        if (is_string) {
            vt->str_values[index] = strdup(str_val);
        } else {
            vt->int_values[index] = int_val;
        }
    } else {
        // Add new variable
        if (vt->count >= vt->capacity) {
            vt->capacity *= 2;
            vt->names = realloc(vt->names, vt->capacity * sizeof(char*));
            vt->int_values = realloc(vt->int_values, vt->capacity * sizeof(int));
            vt->str_values = realloc(vt->str_values, vt->capacity * sizeof(char*));
            vt->is_string = realloc(vt->is_string, vt->capacity * sizeof(int));
        }
        vt->names[vt->count] = strdup(name);
        vt->int_values[vt->count] = int_val;
        vt->str_values[vt->count] = strdup(str_val);
        vt->is_string[vt->count] = is_string;
        vt->count++;
    }
}

void get_var(VariableTable *vt, const char *name, int *out_int_val, char **out_str_val, int *out_is_string) {
    int index = find_var(vt, name);
    if (index >= 0) {
        *out_is_string = vt->is_string[index];
        if (vt->is_string[index]) {
            *out_str_val = vt->str_values[index];
        } else {
            *out_int_val = vt->int_values[index];
        }
    } else {
        // Variable not found, default to 0/empty
        *out_is_string = 0;
        *out_int_val = 0;
        *out_str_val = strdup("");
    }
}

// Simple VM (mimics the Python addxrun.py run function)
void run(InstructionArray *ops) {
    // Simple stack for integer values
    int *int_stack = NULL;
    size_t int_stack_capacity = 0;
    size_t int_stack_size = 0;
    
    // Simple stack for string values (we'll use char* for strings)
    char **str_stack = NULL;
    size_t str_stack_capacity = 0;
    size_t str_stack_size = 0;
    
    // Simple variable storage (name -> value)
    VariableTable vars;
    init_variable_table(&vars);
    
    // Variables for binary operations (declared here to avoid redefinition errors)
    int a, b;
    
    size_t pc = 0;
    while (pc < ops->count) {
        Instruction instr = ops->instructions[pc];
        
        switch (instr.op) {
            case OP_LOAD:
                if (instr.is_string) {
                    // Push string onto string stack
                    if (str_stack_size >= str_stack_capacity) {
                        str_stack_capacity = str_stack_capacity ? str_stack_capacity * 2 : 16;
                        str_stack = realloc(str_stack, str_stack_capacity * sizeof(char*));
                    }
                    str_stack[str_stack_size++] = instr.arg.str_val;
                } else {
                    // Push integer onto int stack
                    if (int_stack_size >= int_stack_capacity) {
                        int_stack_capacity = int_stack_capacity ? int_stack_capacity * 2 : 16;
                        int_stack = realloc(int_stack, int_stack_capacity * sizeof(int));
                    }
                    int_stack[int_stack_size++] = instr.arg.int_val;
                }
                break;
                
            case OP_STORE:
                if (instr.is_string) {
                    // Pop string from string stack and store in variable
                    if (str_stack_size == 0) {
                        fprintf(stderr, "Error: string stack underflow on STORE\n");
                        exit(1);
                    }
                    char *value = str_stack[--str_stack_size];
                    set_var(&vars, instr.arg.str_val, 0, value, 1);
                } else {
                    // Pop integer from int stack and store in variable
                    if (int_stack_size == 0) {
                        fprintf(stderr, "Error: int stack underflow on STORE\n");
                        exit(1);
                    }
                    int value = int_stack[--int_stack_size];
                    set_var(&vars, instr.arg.str_val, value, NULL, 0);
                }
                break;
                
            case OP_PRINT:
                if (instr.is_string) {
                    // Print string
                    if (str_stack_size == 0) {
                        fprintf(stderr, "Error: string stack underflow on PRINT\n");
                        exit(1);
                    }
                    char *value = str_stack[--str_stack_size];
                    printf("%s\n", value);
                } else {
                    // Print integer or variable
                    if (int_stack_size == 0) {
                        fprintf(stderr, "Error: int stack underflow on PRINT\n");
                        exit(1);
                    }
                    int value = int_stack[--int_stack_size];
                    
                    // Check if it's a variable name (string) that we should look up
                    // In this simple implementation, we treat the integer as a direct value
                    // For a more complete implementation, we'd need to distinguish
                    // between integer literals and variable names in the arg
                    // But for simplicity, we'll just print the integer directly
                    printf("%d\n", value);
                }
                break;
                
            case OP_ADD:
                if (int_stack_size < 2) {
                    fprintf(stderr, "Error: int stack underflow on ADD\n");
                    exit(1);
                }
                b = int_stack[--int_stack_size];
                a = int_stack[--int_stack_size];
                
                if (int_stack_size >= int_stack_capacity) {
                    int_stack_capacity = int_stack_capacity ? int_stack_capacity * 2 : 16;
                    int_stack = realloc(int_stack, int_stack_capacity * sizeof(int));
                }
                int_stack[int_stack_size++] = a + b;
                break;
                
            case OP_SUB:
                if (int_stack_size < 2) {
                    fprintf(stderr, "Error: int stack underflow on SUB\n");
                    exit(1);
                }
                b = int_stack[--int_stack_size];
                a = int_stack[--int_stack_size];
                
                if (int_stack_size >= int_stack_capacity) {
                    int_stack_capacity = int_stack_capacity ? int_stack_capacity * 2 : 16;
                    int_stack = realloc(int_stack, int_stack_capacity * sizeof(int));
                }
                int_stack[int_stack_size++] = a - b;
                break;
                
            case OP_MUL:
                if (int_stack_size < 2) {
                    fprintf(stderr, "Error: int stack underflow on MUL\n");
                    exit(1);
                }
                b = int_stack[--int_stack_size];
                a = int_stack[--int_stack_size];
                
                if (int_stack_size >= int_stack_capacity) {
                    int_stack_capacity = int_stack_capacity ? int_stack_capacity * 2 : 16;
                    int_stack = realloc(int_stack, int_stack_capacity * sizeof(int));
                }
                int_stack[int_stack_size++] = a * b;
                break;
                
            case OP_DIV:
                if (int_stack_size < 2) {
                    fprintf(stderr, "Error: int stack underflow on DIV\n");
                    exit(1);
                }
                b = int_stack[--int_stack_size];
                a = int_stack[--int_stack_size];
                if (b == 0) {
                    fprintf(stderr, "Error: division by zero\n");
                    exit(1);
                }
                
                if (int_stack_size >= int_stack_capacity) {
                    int_stack_capacity = int_stack_capacity ? int_stack_capacity * 2 : 16;
                    int_stack = realloc(int_stack, int_stack_capacity * sizeof(int));
                }
                int_stack[int_stack_size++] = a / b;
                break;
                
            case OP_JMP:
                pc = instr.arg.int_val;
                continue; // Skip pc increment
                
            case OP_JZ:
                if (int_stack_size == 0) {
                    fprintf(stderr, "Error: int stack underflow on JZ\n");
                    exit(1);
                }
                b = int_stack[--int_stack_size]; // Reusing b for the condition value
                if (b == 0) {
                    pc = instr.arg.int_val;
                    continue; // Skip pc increment
                }
                break;
                
            case OP_HLT:
                goto cleanup;
        }
        pc++;
    }
    
cleanup:
    // Clean up stacks
    free(int_stack);
    for (size_t i = 0; i < str_stack_size; i++) {
        free(str_stack[i]);
    }
    free(str_stack);
    
    // Clean up variables
    free_variable_table(&vars);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.addx>\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);
    
    printf("Source:\n%s\n---\n", source);
    printf("Starting simple assemble and run...\n");
    
    InstructionArray ops = assemble(source);
    run(&ops);
    
    free_instruction_array(&ops);
    free(source);
    
    return 0;
}