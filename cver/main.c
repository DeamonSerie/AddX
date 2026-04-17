#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fast_parse.h"
#include "compiler.h"
#include "vm.h"
#include "jit.h"

static ASTNode* collect_imports(ASTNode* ast, int depth);

static ASTNode* load_module(const char* module_name, int depth) {
    char path[256];
    int len = strlen(module_name);
    if (len > 250) len = 250;
    memcpy(path, module_name, len);
    memcpy(path + len, ".addx", 6);
    
    printf("[MAIN] Loading module: %s (depth=%d)\n", module_name, depth);
    
    FILE* mf = fopen(path, "r");
    if (!mf) {
        fprintf(stderr, "[MAIN] Cannot find module file: %s\n", path);
        return NULL;
    }
    
    fseek(mf, 0, SEEK_END);
    long msize = ftell(mf);
    fseek(mf, 0, SEEK_SET);
    
    char* msource = malloc(msize + 1);
    fread(msource, 1, msize, mf);
    msource[msize] = '\0';
    fclose(mf);
    
    ASTNode* module_ast = fast_parse(msource);
    free(msource);
    
    return collect_imports(module_ast, depth + 1);
}

static ASTNode* collect_imports(ASTNode* ast, int depth) {
    if (!ast || ast->type != AST_PROGRAM) return ast;
    if (depth > 5) {
        fprintf(stderr, "[MAIN] Import depth exceeds limit\n");
        return ast;
    }
    
    ProgramData* data = ast->data;
    if (!data) return ast;
    
    ASTNode** all_statements = NULL;
    size_t total_count = 0;
    size_t cap = 16;
    all_statements = malloc(sizeof(ASTNode*) * cap);
    
    for (size_t i = 0; i < data->count; i++) {
        ASTNode* stmt = data->statements[i];
        
        if (stmt && stmt->type == AST_IMPORT) {
            ImportData* id = stmt->data;
            if (id && id->module_name) {
                printf("[MAIN] Found import: %s\n", id->module_name);
                
                ASTNode* module_ast = load_module(id->module_name, depth);
                if (module_ast && module_ast->type == AST_PROGRAM) {
                    ProgramData* mdata = module_ast->data;
                    for (size_t j = 0; j < mdata->count; j++) {
                        if (total_count >= cap) { cap *= 2; all_statements = realloc(all_statements, sizeof(ASTNode*) * cap); }
                        all_statements[total_count++] = mdata->statements[j];
                    }
                    free(mdata->statements);
                    free(mdata);
                    free(module_ast);
                }
                continue;
            }
        }
        
        if (total_count >= cap) { cap *= 2; all_statements = realloc(all_statements, sizeof(ASTNode*) * cap); }
        all_statements[total_count++] = stmt;
    }
    
    data->statements = all_statements;
    data->count = total_count;
    
    return ast;
}

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
    
    printf("Processing imports...\n");
    ast = collect_imports(ast, 0);
    printf("Imports processed\n");
    
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