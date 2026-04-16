#include "avm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Helper function to double capacity
static void* double_capacity(void* ptr, size_t* capacity, size_t elem_size) {
    *capacity = (*capacity == 0) ? 16 : *capacity * 2;
    return realloc(ptr, *capacity * elem_size);
}

// Forward declarations for helper functions
static int find_global(char** names, double* values, char** str_values, int* is_string, size_t count, const char* name);
static void set_global(char*** names, double** values, char*** str_values, int** is_string, size_t* count, size_t* capacity, 
                      const char* name, double num_val, const char* str_val, int is_string_val);
static void get_global(char** names, double* values, char** str_values, int* is_string, size_t count, 
                      const char* name, double* out_num, char** out_str, int* out_is_string);

AVM* avm_create(void) {
    AVM* avm = malloc(sizeof(AVM));
    if (!avm) return NULL;
    
    avm->code = NULL;
    avm->code_size = 0;
    avm->code_capacity = 0;
    
    avm->functions = NULL;
    avm->function_count = 0;
    avm->function_capacity = 0;
    
    avm->classes = NULL;
    avm->class_count = 0;
    avm->class_capacity = 0;
    
    avm->global_names = NULL;
    avm->global_values = NULL;
    avm->global_str_values = NULL;
    avm->global_is_string = NULL;
    avm->global_count = 0;
    avm->global_capacity = 0;
    
    return avm;
}

void avm_destroy(AVM* avm) {
    if (!avm) return;
    
    // Free code
    for (size_t i = 0; i < avm->code_size; i++) {
        free(avm->code[i].str_arg);
    }
    free(avm->code);
    
    // Free functions
    for (size_t i = 0; i < avm->function_count; i++) {
        free(avm->functions[i].name);
        free(avm->functions[i].instructions);
    }
    free(avm->functions);
    
    // Free classes (simple for now)
    for (size_t i = 0; i < avm->class_count; i++) {
        free(avm->classes[i].name);
        free(avm->classes[i].super_class);
    }
    free(avm->classes);
    
    // Free globals
    for (size_t i = 0; i < avm->global_count; i++) {
        free(avm->global_names[i]);
        if (avm->global_is_string[i]) {
            free(avm->global_str_values[i]);
        }
    }
    free(avm->global_names);
    free(avm->global_values);
    free(avm->global_str_values);
    free(avm->global_is_string);
    
    free(avm);
}

// Simple stack implementation for the VM
typedef struct {
    double* data;
    size_t size;
    size_t capacity;
} DoubleStack;

typedef struct {
    char** data;
    size_t size;
    size_t capacity;
} StringStack;

static void dstack_init(DoubleStack* s) {
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
}

static void dstack_push(DoubleStack* s, double value) {
    if (s->size >= s->capacity) {
        s->capacity = (s->capacity == 0) ? 16 : s->capacity * 2;
        s->data = realloc(s->data, s->capacity * sizeof(double));
    }
    s->data[s->size++] = value;
}

static double dstack_pop(DoubleStack* s) {
    if (s->size == 0) {
        fprintf(stderr, "Error: stack underflow\n");
        exit(1);
    }
    return s->data[--s->size];
}

static void sstack_init(StringStack* s) {
    s->data = NULL;
    s->size = 0;
    s->capacity = 0;
}

static void sstack_push(StringStack* s, char* value) {
    if (s->size >= s->capacity) {
        s->capacity = (s->capacity == 0) ? 16 : s->capacity * 2;
        s->data = realloc(s->data, s->capacity * sizeof(char*));
    }
    s->data[s->size++] = value;
}

static char* sstack_pop(StringStack* s) {
    if (s->size == 0) {
        fprintf(stderr, "Error: string stack underflow\n");
        exit(1);
    }
    return s->data[--s->size];
}

// Helper functions for global variables
static int find_global(char** names, double* values, char** str_values, int* is_string, size_t count, const char* name) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(names[i], name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static void set_global(char*** names, double** values, char*** str_values, int** is_string, size_t* count, size_t* capacity, 
                       const char* name, double num_val, const char* str_val, int is_string_val) {
    int idx = find_global(*names, *values, *str_values, *is_string, *count, name);
    if (idx >= 0) {
        // Update existing
        if ((*is_string)[idx]) {
            free((*str_values)[idx]);
        }
        (*is_string)[idx] = is_string_val;
        if (is_string_val) {
            (*str_values)[idx] = strdup(str_val);
        } else {
            (*values)[idx] = num_val;
        }
    } else {
        // Add new
        if (*count >= *capacity) {
            *capacity = (*capacity == 0) ? 16 : *capacity * 2;
            *names = realloc(*names, *capacity * sizeof(char*));
            *values = realloc(*values, *capacity * sizeof(double));
            *str_values = realloc(*str_values, *capacity * sizeof(char*));
            *is_string = realloc(*is_string, *capacity * sizeof(int));
        }
        (*names)[*count] = strdup(name);
        (*values)[*count] = num_val;
        (*str_values)[*count] = strdup(str_val);
        (*is_string)[*count] = is_string_val;
        (*count)++;
    }
}

static void get_global(char** names, double* values, char** str_values, int* is_string, size_t count, 
                      const char* name, double* out_num, char** out_str, int* out_is_string) {
    int idx = find_global(names, values, str_values, is_string, count, name);
    if (idx >= 0) {
        *out_is_string = is_string[idx];
        if (is_string[idx]) {
            *out_str = strdup(str_values[idx]);
        } else {
            *out_num = values[idx];
        }
    } else {
        // Not found, default to 0/empty
        *out_is_string = 0;
        *out_num = 0.0;
        *out_str = strdup("");
    }
}

int avm_load_bytecode(AVM* avm, Instruction* code, size_t code_size,
                      Function* functions, size_t function_count,
                      Class* classes, size_t class_count) {
    // Copy code
    avm->code = malloc(code_size * sizeof(Instruction));
    if (!avm->code) return 0;
    memcpy(avm->code, code, code_size * sizeof(Instruction));
    avm->code_size = code_size;
    avm->code_capacity = code_size;
    
    // Copy functions
    avm->functions = malloc(function_count * sizeof(Function));
    if (!avm->functions) {
        free(avm->code);
        return 0;
    }
    memcpy(avm->functions, functions, function_count * sizeof(Function));
    avm->function_count = function_count;
    avm->function_capacity = function_count;
    
    // Note: We're copying the pointers in the functions (name, instructions).
    // The caller must ensure these remain valid for the lifetime of the AVM.
    // For simplicity in this example, we assume the caller manages memory.
    
    // Copy classes
    avm->classes = malloc(class_count * sizeof(Class));
    if (!avm->classes) {
        free(avm->code);
        free(avm->functions);
        return 0;
    }
    memcpy(avm->classes, classes, class_count * sizeof(Class));
    avm->class_count = class_count;
    avm->class_capacity = class_count;
    
    return 1;
}

int avm_run(AVM* avm) {
    // Stacks for execution
    DoubleStack dstack;
    StringStack sstack;
    dstack_init(&dstack);
    sstack_init(&sstack);
    
    // Simple global variable storage (name -> value)
    // We'll use a simple linear search for now (not efficient but simple)
    char** global_names = NULL;
    double* global_values = NULL;
    char** global_str_values = NULL;
    int* global_is_string = NULL;
    size_t global_count = 0;
    size_t global_capacity = 0;
    
    // Program counter
    size_t pc = 0;
    
    while (pc < avm->code_size) {
        Instruction instr = avm->code[pc];
        
        switch (instr.opcode) {
            case OP_LOAD_CONST:
                dstack_push(&dstack, instr.num_arg);
                break;
                

                
            case OP_LOAD_LOCAL:
                // For simplicity, we'll treat locals as globals for now
                // In a real VM, we'd have a stack of frames
                // This is a limitation of this simple implementation
                fprintf(stderr, "Warning: OP_LOAD_LOCAL not fully implemented (using global)\n");
                // Fall through to global load
            case OP_LOAD_GLOBAL:
                {
                    double num_val;
                    char* str_val;
                    int is_string;
                    get_global(global_names, global_values, global_str_values, global_is_string, global_count, 
                              instr.str_arg, &num_val, &str_val, &is_string);
                    if (is_string) {
                        sstack_push(&sstack, str_val);
                    } else {
                        dstack_push(&dstack, num_val);
                    }
                    free(str_val);
                }
                break;
                
            case OP_STORE_GLOBAL: {
                if (sstack.size > 0) {
                    // String value
                    char* val = sstack_pop(&sstack);
                    set_global(&global_names, &global_values, &global_str_values, &global_is_string, 
                              &global_count, &global_capacity, instr.str_arg, 0.0, val, 1);
                } else if (dstack.size > 0) {
                    // Numeric value
                    double val = dstack_pop(&dstack);
                    set_global(&global_names, &global_values, &global_str_values, &global_is_string, 
                              &global_count, &global_capacity, instr.str_arg, val, NULL, 0);
                } else {
                    fprintf(stderr, "Error: stack underflow on STORE_GLOBAL\n");
                    exit(1);
                }
                break;
            }
                

                
            case OP_POP:
                if (sstack.size > 0) {
                    free(sstack_pop(&sstack));
                } else if (dstack.size > 0) {
                    dstack_pop(&dstack);
                } else {
                    fprintf(stderr, "Error: stack underflow on POP\n");
                    exit(1);
                }
                break;
                
            case OP_DUP:
                if (sstack.size > 0) {
                    char* val = sstack_pop(&sstack);
                    sstack_push(&sstack, val);
                    sstack_push(&sstack, val);
                } else if (dstack.size > 0) {
                    double val = dstack_pop(&dstack);
                    dstack_push(&dstack, val);
                    dstack_push(&dstack, val);
                } else {
                    fprintf(stderr, "Error: stack underflow on DUP\n");
                    exit(1);
                }
                break;
                
            case OP_SWAP: {
                if (sstack.size >= 2) {
                    char* a = sstack_pop(&sstack);
                    char* b = sstack_pop(&sstack);
                    sstack_push(&sstack, a);
                    sstack_push(&sstack, b);
                } else if (dstack.size >= 2) {
                    double a = dstack_pop(&dstack);
                    double b = dstack_pop(&dstack);
                    dstack_push(&dstack, a);
                    dstack_push(&dstack, b);
                } else {
                    fprintf(stderr, "Error: stack underflow on SWAP (need 2 values)\n");
                    exit(1);
                }
                break;
            }
                
            case OP_ADD: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on ADD (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, a + b);
                break;
            }
                
            case OP_SUB: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on SUB (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, a - b);
                break;
            }
                
            case OP_MUL: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on MUL (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, a * b);
                break;
            }
                
            case OP_DIV: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on DIV (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                if (b == 0.0) {
                    fprintf(stderr, "Error: division by zero\n");
                    exit(1);
                }
                dstack_push(&dstack, a / b);
                break;
            }
                
            case OP_MOD: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on MOD (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                if (b == 0.0) {
                    fprintf(stderr, "Error: modulus by zero\n");
                    exit(1);
                }
                dstack_push(&dstack, (double)((int)a % (int)b));
                break;
            }
                
            case OP_NEG: {
                if (dstack.size < 1) {
                    fprintf(stderr, "Error: stack underflow on NEG (need 1 value)\n");
                    exit(1);
                }
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, -a);
                break;
            }
                
            case OP_EQ: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on EQ (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, (double)(a == b));
                break;
            }
                
            case OP_NE: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on NE (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, (double)(a != b));
                break;
            }
                
            case OP_LT: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on LT (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, (double)(a < b));
                break;
            }
                
            case OP_GT: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on GT (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, (double)(a > b));
                break;
            }
                
            case OP_LE: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on LE (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, (double)(a <= b));
                break;
            }
                
            case OP_GE: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on GE (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, (double)(a >= b));
                break;
            }
                
            case OP_AND: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on AND (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, (double)((int)a && (int)b));
                break;
            }
                
            case OP_OR: {
                if (dstack.size < 2) {
                    fprintf(stderr, "Error: stack underflow on OR (need 2 values)\n");
                    exit(1);
                }
                double b = dstack_pop(&dstack);
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, (double)((int)a || (int)b));
                break;
            }
                
            case OP_NOT: {
                if (dstack.size < 1) {
                    fprintf(stderr, "Error: stack underflow on NOT (need 1 value)\n");
                    exit(1);
                }
                double a = dstack_pop(&dstack);
                dstack_push(&dstack, (double)(!(int)a));
                break;
            }
                
            case OP_JUMP: {
                size_t target = (size_t)instr.num_arg;
                if (target >= avm->code_size) {
                    fprintf(stderr, "Error: JUMP target out of bounds\n");
                    exit(1);
                }
                pc = target;
                continue; // Skip incrementing pc at the end
            }
                
            case OP_JUMP_IF_FALSE: {
                if (dstack.size < 1) {
                    fprintf(stderr, "Error: stack underflow on JUMP_IF_FALSE\n");
                    exit(1);
                }
                double cond = dstack_pop(&dstack);
                if ((int)cond == 0) {
                    size_t target = (size_t)instr.num_arg;
                    if (target >= avm->code_size) {
                        fprintf(stderr, "Error: JUMP_IF_FALSE target out of bounds\n");
                        exit(1);
                    }
                    pc = target;
                    continue;
                }
                break;
            }
                
            case OP_JUMP_IF_TRUE: {
                if (dstack.size < 1) {
                    fprintf(stderr, "Error: stack underflow on JUMP_IF_TRUE\n");
                    exit(1);
                }
                double cond = dstack_pop(&dstack);
                if ((int)cond != 0) {
                    size_t target = (size_t)instr.num_arg;
                    if (target >= avm->code_size) {
                        fprintf(stderr, "Error: JUMP_IF_TRUE target out of bounds\n");
                        exit(1);
                    }
                    pc = target;
                    continue;
                }
                break;
            }
                
            case OP_CALL: {
                // In a full implementation, we would push a call frame
                // For this simple AVM, we'll just note that function calls
                // are not implemented in this basic version
                fprintf(stderr, "Warning: OP_CALL not implemented\n");
                // Skip for now
                break;
            }
                
            case OP_RETURN:
            case OP_RETURN_VAL:
                // Return from function - in a simple script without functions,
                // we just halt
                // For a full implementation, we'd pop the call frame
                fprintf(stderr, "Warning: OP_RETURN not fully implemented\n");
                // For now, treat as halt
                goto cleanup;
                
            case OP_MAKE_LIST: {
                // In a simple implementation, we'll just push a placeholder
                // A full implementation would create a list object
                fprintf(stderr, "Warning: OP_MAKE_LIST not implemented\n");
                dstack_push(&dstack, 0.0); // Push null/list id
                break;
            }
                
            case OP_MAKE_DICT: {
                fprintf(stderr, "Warning: OP_MAKE_DICT not implemented\n");
                dstack_push(&dstack, 0.0);
                break;
            }
                
            case OP_LIST_GET:
            case OP_LIST_SET:
            case OP_LIST_APPEND:
            case OP_DICT_GET:
            case OP_DICT_SET:
            case OP_DICT_HAS:
                fprintf(stderr, "Warning: data structure op not implemented\n");
                dstack_push(&dstack, 0.0);
                break;
                
            case OP_NEW: {
                fprintf(stderr, "Warning: OP_NEW not implemented\n");
                dstack_push(&dstack, 0.0); // Push null object id
                break;
            }
                
            case OP_DELETE: {
                fprintf(stderr, "Warning: OP_DELETE not implemented\n");
                // Pop the object reference (if any)
                if (sstack.size > 0) {
                    free(sstack_pop(&sstack));
                } else if (dstack.size > 0) {
                    dstack_pop(&dstack);
                }
                break;
            }
                
            case OP_GET_ATTR: {
                fprintf(stderr, "Warning: OP_GET_ATTR not implemented\n");
                // Pop object, push attribute value (placeholder)
                if (sstack.size > 0) {
                    free(sstack_pop(&sstack));
                } else if (dstack.size > 0) {
                    dstack_pop(&dstack);
                }
                dstack_push(&dstack, 0.0);
                break;
            }
                
            case OP_SET_ATTR: {
                fprintf(stderr, "Warning: OP_SET_ATTR not implemented\n");
                // Pop value, object, and attribute name (we don't have attribute name here)
                // In a full implementation, the attribute name would be in the instruction
                if (sstack.size > 0) {
                    free(sstack_pop(&sstack)); // value
                } else if (dstack.size > 0) {
                    dstack_pop(&dstack); // value
                }
                if (sstack.size > 0) {
                    free(sstack_pop(&sstack)); // object
                } else if (dstack.size > 0) {
                    dstack_pop(&dstack); // object
                }
                // Attribute name would be in instr.str_arg, but we ignore for now
                break;
            }
                
            case OP_METHOD_CALL: {
                fprintf(stderr, "Warning: OP_METHOD_CALL not implemented\n");
                // Pop arguments, object, and method name
                // We don't have the argument count here in a simple implementation
                // For now, just clear the stack a bit
                if (sstack.size > 0) {
                    free(sstack_pop(&sstack)); // placeholder for result
                } else if (dstack.size > 0) {
                    dstack_pop(&dstack);
                }
                break;
            }
                
            case OP_ADDRESS_OF: {
                fprintf(stderr, "Warning: OP_ADDRESS_OF not implemented\n");
                dstack_push(&dstack, 0.0); // Push null address
                break;
            }
                
            case OP_DEREFERENCE: {
                fprintf(stderr, "Warning: OP_DEREFERENCE not implemented\n");
                if (dstack.size < 1) {
                    fprintf(stderr, "Error: stack underflow on DEREFERENCE\n");
                    exit(1);
                }
                // Pop address, push value at address (placeholder)
                dstack_pop(&dstack);
                dstack_push(&dstack, 0.0);
                break;
            }
                
            case OP_SIZEOF: {
                fprintf(stderr, "Warning: OP_SIZEOF not implemented\n");
                dstack_push(&dstack, 1.0); // Assume size 1 for now
                break;
            }
                
            case OP_PRINT: {
                if (sstack.size > 0) {
                    char* val = sstack_pop(&sstack);
                    printf("%s\n", val);
                    free(val);
                } else if (dstack.size > 0) {
                    double val = dstack_pop(&dstack);
                    // If it's an integer-like value, print as integer
                    if (val == (double)((int)val)) {
                        printf("%d\n", (int)val);
                    } else {
                        printf("%f\n", val);
                    }
                } else {
                    fprintf(stderr, "Error: stack underflow on PRINT\n");
                    exit(1);
                }
                break;
            }
                
            case OP_HLT:
                goto cleanup;
                
            default:
                fprintf(stderr, "Error: unknown opcode %d\n", instr.opcode);
                exit(1);
        }
        
        pc++;
    }
    
cleanup:
    // Clean up stacks
    free(dstack.data);
    while (sstack.size > 0) {
        free(sstack_pop(&sstack));
    }
    free(sstack.data);
    
    // Clean up global variables
    for (size_t i = 0; i < global_count; i++) {
        free(global_names[i]);
        if (global_is_string[i]) {
            free(global_str_values[i]);
        }
    }
    free(global_names);
    free(global_values);
    free(global_str_values);
    free(global_is_string);
    
    return 0;
}

void avm_print_state(const AVM* avm) {
    printf("AVM State:\n");
    printf("  Code size: %zu instructions\n", avm->code_size);
    printf("  Function count: %zu\n", avm->function_count);
    printf("  Class count: %zu\n", avm->class_count);
    printf("  Global count: %zu\n", avm->global_count);
    
    // Print first few instructions
    printf("  First 10 instructions:\n");
    for (size_t i = 0; i < avm->code_size && i < 10; i++) {
        printf("    %zu: ", i);
        switch (avm->code[i].opcode) {
            case OP_LOAD_CONST: printf("LOAD_CONST %f\n", avm->code[i].num_arg); break;
            case OP_LOAD_GLOBAL: printf("LOAD_GLOBAL %s\n", avm->code[i].str_arg ? avm->code[i].str_arg : "NULL"); break;
            case OP_LOAD_LOCAL: printf("LOAD_LOCAL %d\n", (int)avm->code[i].num_arg); break;
            case OP_STORE_GLOBAL: printf("STORE_GLOBAL %s\n", avm->code[i].str_arg ? avm->code[i].str_arg : "NULL"); break;
            case OP_STORE_LOCAL: printf("STORE_LOCAL %d\n", (int)avm->code[i].num_arg); break;
            case OP_POP: printf("POP\n"); break;
            case OP_DUP: printf("DUP\n"); break;
            case OP_SWAP: printf("SWAP\n"); break;
            case OP_ADD: printf("ADD\n"); break;
            case OP_SUB: printf("SUB\n"); break;
            case OP_MUL: printf("MUL\n"); break;
            case OP_DIV: printf("DIV\n"); break;
            case OP_MOD: printf("MOD\n"); break;
            case OP_NEG: printf("NEG\n"); break;
            case OP_EQ: printf("EQ\n"); break;
            case OP_NE: printf("NE\n"); break;
            case OP_LT: printf("LT\n"); break;
            case OP_GT: printf("GT\n"); break;
            case OP_LE: printf("LE\n"); break;
            case OP_GE: printf("GE\n"); break;
            case OP_AND: printf("AND\n"); break;
            case OP_OR: printf("OR\n"); break;
            case OP_NOT: printf("NOT\n"); break;
            case OP_JUMP: printf("JUMP %f\n", avm->code[i].num_arg); break;
            case OP_JUMP_IF_FALSE: printf("JUMP_IF_FALSE %f\n", avm->code[i].num_arg); break;
            case OP_JUMP_IF_TRUE: printf("JUMP_IF_TRUE %f\n", avm->code[i].num_arg); break;
            case OP_CALL: printf("CALL %f\n", avm->code[i].num_arg); break;
            case OP_RETURN: printf("RETURN\n"); break;
            case OP_RETURN_VAL: printf("RETURN_VAL\n"); break;
            case OP_MAKE_LIST: printf("MAKE_LIST %f\n", avm->code[i].num_arg); break;
            case OP_MAKE_DICT: printf("MAKE_DICT %f\n", avm->code[i].num_arg); break;
            case OP_LIST_GET: printf("LIST_GET\n"); break;
            case OP_LIST_SET: printf("LIST_SET\n"); break;
            case OP_LIST_APPEND: printf("LIST_APPEND\n"); break;
            case OP_DICT_GET: printf("DICT_GET\n"); break;
            case OP_DICT_SET: printf("DICT_SET\n"); break;
            case OP_DICT_HAS: printf("DICT_HAS\n"); break;
            case OP_NEW: printf("NEW %s\n", avm->code[i].str_arg ? avm->code[i].str_arg : "NULL"); break;
            case OP_DELETE: printf("DELETE %s\n", avm->code[i].str_arg ? avm->code[i].str_arg : "NULL"); break;
            case OP_GET_ATTR: printf("GET_ATTR %s\n", avm->code[i].str_arg ? avm->code[i].str_arg : "NULL"); break;
            case OP_SET_ATTR: printf("SET_ATTR %s\n", avm->code[i].str_arg ? avm->code[i].str_arg : "NULL"); break;
            case OP_METHOD_CALL: printf("METHOD_CALL %s\n", avm->code[i].str_arg ? avm->code[i].str_arg : "NULL"); break;
            case OP_ADDRESS_OF: printf("ADDRESS_OF %s\n", avm->code[i].str_arg ? avm->code[i].str_arg : "NULL"); break;
            case OP_DEREFERENCE: printf("DEREFERENCE\n"); break;
            case OP_SIZEOF: printf("SIZEOF %s\n", avm->code[i].str_arg ? avm->code[i].str_arg : "NULL"); break;
            case OP_PRINT: printf("PRINT\n"); break;
            case OP_HLT: printf("HLT\n"); break;
            default: printf("UNKNOWN %d\n", avm->code[i].opcode); break;
        }
    }
    if (avm->code_size > 10) {
        printf("    ... and %zu more instructions\n", avm->code_size - 10);
    }
}