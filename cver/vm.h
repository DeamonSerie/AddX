#ifndef VM_H
#define VM_H

#include "compiler.h"

typedef struct {
    double* stack;
    size_t stack_size;
    size_t stack_top;
    double* locals;
    double* globals;
    size_t globals_size;
    Function* functions;
    size_t function_count;
} VM;

VM* create_vm(CompiledProgram* prog);
void run_vm(VM* vm, Function* func);
void free_vm(VM* vm);

#endif