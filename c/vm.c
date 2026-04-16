#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"

static double get_global(VM* vm, const char* name) {
    for (size_t i = 0; i < vm->globals_size; i += 2) {
        if (strcmp((char*)(int)vm->globals[i], name) == 0) {
            return vm->globals[i + 1];
        }
    }
    return 0;
}

static void set_global(VM* vm, const char* name, double value) {
    for (size_t i = 0; i < vm->globals_size; i += 2) {
        if (strcmp((char*)(int)vm->globals[i], name) == 0) {
            vm->globals[i + 1] = value;
            return;
        }
    }
    size_t new_size = vm->globals_size + 2;
    vm->globals = realloc(vm->globals, sizeof(double) * new_size);
    vm->globals[vm->globals_size] = (double)(int)strdup(name);
    vm->globals[vm->globals_size + 1] = value;
    vm->globals_size = new_size;
}

VM* create_vm(CompiledProgram* prog) {
    VM* vm = malloc(sizeof(VM));
    vm->stack = malloc(sizeof(double) * 1024);
    vm->stack_size = 1024;
    vm->stack_top = 0;
    vm->locals = NULL;
    vm->globals = malloc(sizeof(double) * 64);
    vm->globals_size = 0;
    vm->functions = prog->functions;
    vm->function_count = prog->function_count;
    return vm;
}

static double pop(VM* vm) {
    if (vm->stack_top == 0) return 0;
    return vm->stack[--vm->stack_top];
}

static void push(VM* vm, double value) {
    if (vm->stack_top >= vm->stack_size) {
        vm->stack_size *= 2;
        vm->stack = realloc(vm->stack, sizeof(double) * vm->stack_size);
    }
    vm->stack[vm->stack_top++] = value;
}

void run_vm(VM* vm, Function* func) {
    printf("Running %s with %zu instructions\n", func->name, func->instruction_count);
    
    vm->locals = calloc(func->param_count, sizeof(double));
    
    for (size_t i = 0; i < func->instruction_count; i++) {
        Instruction* inst = &func->instructions[i];
        
        switch (inst->opcode) {
            case OP_LOAD_CONST:
                push(vm, inst->arg);
                break;
            
            case OP_LOAD_GLOBAL:
                push(vm, get_global(vm, inst->str_arg));
                break;
            
            case OP_STORE_GLOBAL:
                set_global(vm, inst->str_arg, pop(vm));
                break;
            
            case OP_LOAD_LOCAL:
                push(vm, vm->locals[(int)inst->arg]);
                break;
            
            case OP_STORE_LOCAL:
                vm->locals[(int)inst->arg] = pop(vm);
                break;
            
            case OP_POP:
                pop(vm);
                break;
            
            case OP_ADD: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a + b);
                break;
            }
            case OP_SUB: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a - b);
                break;
            }
            case OP_MUL: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a * b);
                break;
            }
            case OP_DIV: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a / b);
                break;
            }
            case OP_MOD: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, (double)((int)a % (int)b));
                break;
            }
            case OP_EQ: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a == b ? 1 : 0);
                break;
            }
            case OP_NE: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a != b ? 1 : 0);
                break;
            }
            case OP_LT: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a < b ? 1 : 0);
                break;
            }
            case OP_GT: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a > b ? 1 : 0);
                break;
            }
            case OP_LE: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a <= b ? 1 : 0);
                break;
            }
            case OP_GE: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a >= b ? 1 : 0);
                break;
            }
            case OP_AND: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, (a != 0 && b != 0) ? 1 : 0);
                break;
            }
            case OP_OR: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, (a != 0 || b != 0) ? 1 : 0);
                break;
            }
            case OP_NOT: {
                double a = pop(vm);
                push(vm, a == 0 ? 1 : 0);
                break;
            }
            case OP_JUMP:
                i = (size_t)(inst->arg - 1);
                break;
            
            case OP_JUMP_IF_FALSE: {
                double cond = pop(vm);
                if (cond == 0) {
                    i = (size_t)(inst->arg - 1);
                }
                break;
            }
            
            case OP_JUMP_IF_TRUE: {
                double cond = pop(vm);
                if (cond != 0) {
                    i = (size_t)(inst->arg - 1);
                }
                break;
            }
            
            case OP_CALL: {
                int arg_count = (int)inst->arg;
                double args[16];
                for (int j = arg_count - 1; j >= 0; j--) {
                    args[j] = pop(vm);
                }
                
                for (size_t f = 0; f < vm->function_count; f++) {
                    if (strcmp(vm->functions[f].name, inst->str_arg) == 0) {
                        run_vm(vm, &vm->functions[f]);
                        break;
                    }
                }
                break;
            }
            
            case OP_RETURN:
                free(vm->locals);
                return;
            
            case OP_PRINT: {
                int count = (int)inst->arg;
                for (int j = count - 1; j >= 0; j--) {
                    printf("%.2f", vm->stack[vm->stack_top - 1 - j]);
                }
                break;
            }
            
            case OP_MAKE_LIST: {
                int count = (int)inst->arg;
                break;
            }
            
            case OP_LIST_GET: {
                double idx = pop(vm);
                double lst = pop(vm);
                push(vm, 0);
                break;
            }
            
            case OP_GET_ATTR: {
                pop(vm);
                push(vm, 0);
                break;
            }
            
            case OP_NEW: {
                push(vm, 0);
                break;
            }
            
            case OP_SIZEOF: {
                if (inst->str_arg) {
                    if (strcmp(inst->str_arg, "int") == 0) push(vm, 4);
                    else if (strcmp(inst->str_arg, "float") == 0) push(vm, 4);
                    else if (strcmp(inst->str_arg, "double") == 0) push(vm, 8);
                    else if (strcmp(inst->str_arg, "char") == 0) push(vm, 1);
                    else push(vm, 4);
                } else {
                    pop(vm);
                    push(vm, 4);
                }
                break;
            }
            
            case OP_HALT:
                free(vm->locals);
                return;
            
            default:
                break;
        }
    }
    
    free(vm->locals);
}

void free_vm(VM* vm) {
    free(vm->stack);
    free(vm->locals);
    free(vm->globals);
    free(vm);
}