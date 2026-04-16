#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fast_parse.h"
#include "compiler.h"
#include "vm.h"
#include "jit.h"

int main(int argc, char** argv) {
    setbuf(stdout, NULL);
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.addx> [--jit]\n", argv[0]);
        return 1;
    }
    
    int use_jit = 0;
    const char* filename = argv[1];
    if (argc > 2 && strcmp(argv[2], "--jit") == 0) {
        use_jit = 1;
    }
    
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);
    
    printf("Source:\n%s\n---\n", source);
    printf("Starting parse...\n");
    
    ASTNode* ast = fast_parse(source);
    printf("Parsed\n");
    
    CompiledProgram* prog = compile(ast);
    printf("Compiled\n");
    
    Function* main_func = NULL;
    for (size_t i = 0; i < prog->function_count; i++) {
        printf("Found function: %s\n", prog->functions[i].name);
        if (strcmp(prog->functions[i].name, "main") == 0) {
            main_func = &prog->functions[i];
        }
    }
    
    if (!main_func && prog->instruction_count > 0) {
        main_func = &(Function){
            .name = "main",
            .instructions = prog->instructions,
            .instruction_count = prog->instruction_count,
            .param_count = 0
        };
    }
    
    if (use_jit) {
        printf("\n=== Using JIT Compiler ===\n");
        JITContext* jit = jit_init();
        if (main_func) {
            jit_compile(jit, main_func);
            jit_run(jit);
        }
        jit_free(jit);
    } else {
        printf("\n=== Using VM Interpreter ===\n");
        VM* vm = create_vm(prog);
        if (main_func) {
            run_vm(vm, main_func);
        }
        free_vm(vm);
    }
    
    free_compiled(prog);
    fast_free(ast);
    free(source);
    
    return 0;
}