#ifndef JIT_H
#define JIT_H

#include "compiler.h"

typedef struct {
    unsigned char* code;
    size_t capacity;
    size_t top;
} JITBuffer;

typedef struct {
    JITBuffer buffer;
    double* stack;
    size_t stack_top;
    double* globals;
    size_t global_count;
    char** global_names;
} JITContext;

JITContext* jit_init(void);
void jit_compile(JITContext* ctx, Function* func);
void jit_run(JITContext* ctx);
void jit_free(JITContext* ctx);

#endif