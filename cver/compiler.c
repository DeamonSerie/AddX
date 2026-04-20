#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

typedef struct {
    Instruction* instructions;
    size_t count;
    size_t capacity;
    char** local_vars;
    size_t local_count;
    char** global_vars;
    size_t global_count;
    char** imported_modules;
    size_t imported_count;
} Compiler;

static int is_local(Compiler* c, const char* name) {
    for (size_t i = 0; i < c->local_count; i++) {
        if (strcmp(c->local_vars[i], name) == 0) return 1;
    }
    return 0;
}

static int is_global(Compiler* c, const char* name) {
    for (size_t i = 0; i < c->global_count; i++) {
        if (strcmp(c->global_vars[i], name) == 0) return 1;
    }
    return 0;
}

static void add_local(Compiler* c, const char* name) {
    if (!is_local(c, name) && !is_global(c, name)) {
        c->local_vars = realloc(c->local_vars, sizeof(char*) * (c->local_count + 1));
        c->local_vars[c->local_count++] = strdup(name);
    }
}

static void add_global(Compiler* c, const char* name) {
    if (!is_global(c, name)) {
        c->global_vars = realloc(c->global_vars, sizeof(char*) * (c->global_count + 1));
        c->global_vars[c->global_count++] = strdup(name);
    }
}

static void emit(Compiler* c, OpCode opcode, double arg, char* str_arg) {
    if (c->count >= c->capacity) {
        c->capacity = c->capacity ? c->capacity * 2 : 64;
        c->instructions = realloc(c->instructions, sizeof(Instruction) * c->capacity);
    }
    c->instructions[c->count].opcode = opcode;
    c->instructions[c->count].arg = arg;
    c->instructions[c->count].str_arg = str_arg ? strdup(str_arg) : NULL;
    c->count++;
}

static void compile_node(Compiler* c, ASTNode* node);

static void compile_expression(Compiler* c, ASTNode* node);

static void compile_var_decl(Compiler* c, VarDeclData* data) {
    if (data->value) {
        compile_expression(c, data->value);
        emit(c, OP_STORE_GLOBAL, 0, data->name);
        add_global(c, data->name);
    }
}

static void compile_assignment(Compiler* c, AssignmentData* data) {
    compile_expression(c, data->value);
    if (is_local(c, data->target)) {
        emit(c, OP_STORE_LOCAL, 0, data->target);
    } else {
        emit(c, OP_STORE_GLOBAL, 0, data->target);
        add_global(c, data->target);
    }
}

static void compile_if(Compiler* c, IfData* data) {
    compile_expression(c, data->condition);
    
    size_t jump_pos = c->count;
    emit(c, OP_JUMP_IF_FALSE, 0, NULL);
    
    for (size_t i = 0; i < data->then_count; i++) {
        compile_node(c, data->then_branch[i]);
    }
    
    size_t else_start = c->count;
    emit(c, OP_JUMP, 0, NULL);
    
    c->instructions[jump_pos].arg = (double)(c->count + 1);
    
    if (data->else_branch) {
        for (size_t i = 0; i < data->else_count; i++) {
            compile_node(c, data->else_branch[i]);
        }
    }
    
    c->instructions[else_start].arg = (double)(c->count + 1);
}

static void compile_while(Compiler* c, WhileData* data) {
    size_t loop_start = c->count;
    
    compile_expression(c, data->condition);
    size_t jump_pos = c->count;
    emit(c, OP_JUMP_IF_FALSE, 0, NULL);
    
    for (size_t i = 0; i < data->body_count; i++) {
        compile_node(c, data->body[i]);
    }
    
    emit(c, OP_JUMP, (double)(loop_start + 1), NULL);
    c->instructions[jump_pos].arg = (double)(c->count + 1);
}

static void compile_for(Compiler* c, ForData* data) {
    if (data->start) {
        compile_expression(c, data->start);
    } else {
        emit(c, OP_LOAD_CONST, 0, NULL);
    }
    emit(c, OP_STORE_LOCAL, 0, data->var_name);
    add_local(c, data->var_name);
    
    size_t loop_start = c->count;
    emit(c, OP_LOAD_LOCAL, 0, data->var_name);
    compile_expression(c, data->end);
    emit(c, OP_LT, 0, NULL);
    size_t jump_pos = c->count;
    emit(c, OP_JUMP_IF_FALSE, 0, NULL);
    
    for (size_t i = 0; i < data->body_count; i++) {
        compile_node(c, data->body[i]);
    }
    
    if (data->step) {
        compile_expression(c, data->step);
    } else {
        emit(c, OP_LOAD_CONST, 1, NULL);
    }
    emit(c, OP_ADD, 0, NULL);
    emit(c, OP_STORE_LOCAL, 0, data->var_name);
    
    emit(c, OP_JUMP, (double)(loop_start + 1), NULL);
    c->instructions[jump_pos].arg = (double)(c->count + 1);
}

static void compile_return(Compiler* c, ReturnData* data) {
    if (data->value) {
        compile_expression(c, data->value);
    }
    emit(c, OP_RETURN, 0, NULL);
}

static void compile_print(Compiler* c, PrintData* data) {
    for (size_t i = 0; i < data->arg_count; i++) {
        compile_expression(c, data->args[i]);
    }
    emit(c, OP_PRINT, (double)data->arg_count, NULL);
}

static void compile_static_var(Compiler* c, StaticVarData* data) {
    if (data->value) {
        compile_expression(c, data->value);
    } else {
        emit(c, OP_LOAD_CONST, 0, NULL);
    }
    emit(c, OP_STORE_GLOBAL, 0, data->name);
    add_global(c, data->name);
}

static void compile_const(Compiler* c, ConstData* data) {
    compile_expression(c, data->value);
    emit(c, OP_STORE_GLOBAL, 0, data->name);
    add_global(c, data->name);
}

static double eval_const_expr(ASTNode* node) {
    if (node->type == AST_NUMBER) {
        NumberData* d = node->data;
        return d->value;
    }
    return 0;
}

static void compile_expression(Compiler* c, ASTNode* node) {
    switch (node->type) {
        case AST_NUMBER: {
            NumberData* d = node->data;
            emit(c, OP_LOAD_CONST, d->value, NULL);
            break;
        }
        case AST_STRING: {
            StringData* d = node->data;
            emit(c, OP_LOAD_CONST, 0, d->value);
            break;
        }
        case AST_BOOL: {
            BoolData* d = node->data;
            emit(c, OP_LOAD_CONST, d->value ? 1 : 0, NULL);
            break;
        }
        case AST_NONE:
        case AST_NULLPTR: {
            emit(c, OP_LOAD_CONST, 0, NULL);
            break;
        }
        case AST_IDENTIFIER: {
            IdentifierData* d = node->data;
            if (is_local(c, d->name)) {
                emit(c, OP_LOAD_LOCAL, 0, d->name);
            } else {
                emit(c, OP_LOAD_GLOBAL, 0, d->name);
            }
            break;
        }
        case AST_BINARY_OP: {
            BinaryOpData* d = node->data;
            compile_expression(c, d->left);
            compile_expression(c, d->right);
            if (strcmp(d->op, "+") == 0) emit(c, OP_ADD, 0, NULL);
            else if (strcmp(d->op, "-") == 0) emit(c, OP_SUB, 0, NULL);
            else if (strcmp(d->op, "*") == 0) emit(c, OP_MUL, 0, NULL);
            else if (strcmp(d->op, "/") == 0) emit(c, OP_DIV, 0, NULL);
            else if (strcmp(d->op, "%") == 0) emit(c, OP_MOD, 0, NULL);
            else if (strcmp(d->op, "==") == 0) emit(c, OP_EQ, 0, NULL);
            else if (strcmp(d->op, "!=") == 0) emit(c, OP_NE, 0, NULL);
            else if (strcmp(d->op, "<") == 0) emit(c, OP_LT, 0, NULL);
            else if (strcmp(d->op, ">") == 0) emit(c, OP_GT, 0, NULL);
            else if (strcmp(d->op, "<=") == 0) emit(c, OP_LE, 0, NULL);
            else if (strcmp(d->op, ">=") == 0) emit(c, OP_GE, 0, NULL);
            else if (strcmp(d->op, "and") == 0) emit(c, OP_AND, 0, NULL);
            else if (strcmp(d->op, "or") == 0) emit(c, OP_OR, 0, NULL);
            break;
        }
        case AST_UNARY_OP: {
            UnaryOpData* d = node->data;
            compile_expression(c, d->operand);
            if (strcmp(d->op, "-") == 0) {
                emit(c, OP_LOAD_CONST, -1, NULL);
                emit(c, OP_MUL, 0, NULL);
            } else if (strcmp(d->op, "not") == 0) {
                emit(c, OP_NOT, 0, NULL);
            }
            break;
        }
        case AST_LIST: {
            ListData* d = node->data;
            for (size_t i = 0; i < d->count; i++) {
                compile_expression(c, d->elements[i]);
            }
            emit(c, OP_MAKE_LIST, (double)d->count, NULL);
            break;
        }
        case AST_CALL: {
            CallData* d = node->data;
            for (size_t i = 0; i < d->arg_count; i++) {
                compile_expression(c, d->args[i]);
            }
            if (d->func->type == AST_IDENTIFIER) {
                IdentifierData* id = d->func->data;
                emit(c, OP_CALL, (double)d->arg_count, id->name);
            }
            break;
        }
        case AST_INDEX_ACCESS: {
            IndexAccessData* d = node->data;
            compile_expression(c, d->object);
            compile_expression(c, d->index);
            emit(c, OP_LIST_GET, 0, NULL);
            break;
        }
        case AST_ATTRIBUTE_ACCESS: {
            AttributeAccessData* d = node->data;
            compile_expression(c, d->object);
            emit(c, OP_GET_ATTR, 0, d->attribute);
            break;
        }
        case AST_ADDRESS_OF: {
            IdentifierData* d = node->data;
            emit(c, OP_ADDRESS_OF, 0, d->name);
            break;
        }
        case AST_DEREFERENCE: {
            compile_expression(c, (ASTNode*)node->data);
            emit(c, OP_DEREFERENCE, 0, NULL);
            break;
        }
        case AST_SIZEOF: {
            SizeofData* d = node->data;
            if (d->type_name) {
                emit(c, OP_SIZEOF, 0, d->type_name);
            } else {
                compile_expression(c, d->expr[0]);
                emit(c, OP_SIZEOF, 0, NULL);
            }
            break;
        }
        case AST_NEW: {
            NewData* d = node->data;
            emit(c, OP_NEW, 0, d->type_name);
            break;
        }
        default:
            break;
    }
}

static void compile_node(Compiler* c, ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_FUNCTION: {
            FunctionData* d = node->data;
            Compiler func_comp = {0};
            for (size_t i = 0; i < d->param_count; i++) {
                add_local(&func_comp, d->param_names[i]);
            }
            for (size_t i = 0; i < d->body_count; i++) {
                compile_node(&func_comp, d->body[i]);
            }
            int has_return = 0;
            for (size_t i = 0; i < d->body_count; i++) {
                if (d->body[i] && d->body[i]->type == AST_RETURN) {
                    has_return = 1;
                    break;
                }
            }
            if (!has_return) {
                emit(&func_comp, OP_RETURN, 0, NULL);
            }
            break;
        }
        case AST_VAR_DECL: {
            VarDeclData* d = node->data;
            compile_var_decl(c, d);
            break;
        }
        case AST_ASSIGNMENT: {
            AssignmentData* d = node->data;
            compile_assignment(c, d);
            break;
        }
        case AST_IF: {
            IfData* d = node->data;
            compile_if(c, d);
            break;
        }
        case AST_WHILE: {
            WhileData* d = node->data;
            compile_while(c, d);
            break;
        }
        case AST_FOR: {
            ForData* d = node->data;
            compile_for(c, d);
            break;
        }
        case AST_RETURN: {
            ReturnData* d = node->data;
            compile_return(c, d);
            break;
        }
        case AST_PRINT: {
            PrintData* d = node->data;
            compile_print(c, d);
            break;
        }
        case AST_STATIC_VAR: {
            StaticVarData* d = node->data;
            compile_static_var(c, d);
            break;
        }
        case AST_CONST: {
            ConstData* d = node->data;
            compile_const(c, d);
            break;
        }
        case AST_IMPORT: {
            ImportData* d = node->data;
            if (d && d->module_name) {
                printf("[COMPILE] Importing module: %s\n", d->module_name);
            }
            break;
        }
        default:
            compile_expression(c, node);
            break;
    }
}

CompiledProgram* compile(ASTNode* ast) {
    if (ast->type != AST_PROGRAM) {
        fprintf(stderr, "Expected program node\n");
        return NULL;
    }
    
    ProgramData* data = ast->data;
    CompiledProgram* prog = malloc(sizeof(CompiledProgram));
    prog->instructions = NULL;
    prog->instruction_count = 0;
    prog->functions = NULL;
    prog->function_count = 0;
    prog->classes = NULL;
    prog->class_count = 0;
    
    Compiler global_comp = {0};
    
    for (size_t i = 0; i < data->count; i++) {
        if (data->statements[i]->type == AST_FUNCTION) {
            FunctionData* fd = data->statements[i]->data;
            
            Compiler func_comp = {0};
            for (size_t j = 0; j < fd->param_count; j++) {
                add_local(&func_comp, fd->param_names[j]);
            }
            
            for (size_t j = 0; j < fd->body_count; j++) {
                compile_node(&func_comp, fd->body[j]);
            }
            
            int has_return = 0;
            for (size_t j = 0; j < fd->body_count; j++) {
                if (fd->body[j]->type == AST_RETURN) {
                    has_return = 1;
                    break;
                }
            }
            if (!has_return) {
                emit(&func_comp, OP_RETURN, 0, NULL);
            }
            
            prog->functions = realloc(prog->functions, sizeof(Function) * (prog->function_count + 1));
            Function* func = &prog->functions[prog->function_count++];
            func->name = strdup(fd->name);
            func->param_names = fd->param_names;
            func->param_types = fd->param_types;
            func->param_count = fd->param_count;
            func->return_type = fd->return_type;
            func->instructions = func_comp.instructions;
            func->instruction_count = func_comp.count;
            func->is_static = fd->is_static;
        } else {
            compile_node(&global_comp, data->statements[i]);
        }
    }
    
    if (global_comp.count > 0) {
        emit(&global_comp, OP_HALT, 0, NULL);
        prog->instructions = global_comp.instructions;
        prog->instruction_count = global_comp.count;
    } else if (prog->function_count == 0) {
        emit(&global_comp, OP_HALT, 0, NULL);
        prog->instructions = global_comp.instructions;
        prog->instruction_count = global_comp.count;
    }
    
    return prog;
}

void free_compiled(CompiledProgram* prog) {
    if (!prog) return;
    
    for (size_t i = 0; i < prog->function_count; i++) {
        free(prog->functions[i].name);
        free(prog->functions[i].return_type);
        free(prog->functions[i].instructions);
    }
    free(prog->functions);
    
    for (size_t i = 0; i < prog->class_count; i++) {
        free(prog->classes[i].name);
    }
    free(prog->classes);
    
    free(prog->instructions);
    free(prog);
}