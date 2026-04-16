#ifndef COMPILER_H
#define COMPILER_H

#include "fast_parse.h"

typedef enum {
    OP_LOAD_CONST,
    OP_LOAD_GLOBAL,
    OP_STORE_GLOBAL,
    OP_LOAD_LOCAL,
    OP_STORE_LOCAL,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    OP_CALL,
    OP_RETURN,
    OP_PRINT,
    OP_MAKE_LIST,
    OP_MAKE_DICT,
    OP_LIST_GET,
    OP_LIST_SET,
    OP_DICT_GET,
    OP_NEW,
    OP_GET_ATTR,
    OP_SET_ATTR,
    OP_ADDRESS_OF,
    OP_DEREFERENCE,
    OP_SIZEOF,
    OP_HALT
} OpCode;

typedef struct {
    OpCode opcode;
    double arg;
    char* str_arg;
} Instruction;

typedef struct {
    char* name;
    char** param_names;
    char** param_types;
    size_t param_count;
    char* return_type;
    Instruction* instructions;
    size_t instruction_count;
} Function;

typedef struct {
    char* name;
    Function* methods;
    size_t method_count;
} Class;

typedef struct {
    Instruction* instructions;
    size_t instruction_count;
    Function* functions;
    size_t function_count;
    Class* classes;
    size_t class_count;
} CompiledProgram;

CompiledProgram* compile(ASTNode* ast);
void free_compiled(CompiledProgram* prog);

#endif