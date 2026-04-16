#include <stdio.h>
#include <stdlib.h>
#include "avm.h"
#include "tokenizer.h"
#include "parser.h"
#include "ast.h"

// Simple test to create bytecode for: print(42);
/*
int test_avm_main() {
    printf("Testing AVM with simple print program...\n");
    
    // Create a simple instruction array manually for: print(42);
    // This would be: LOAD_CONST 42.0, PRINT, HLT
    
    AVM* avm = avm_create();
    if (!avm) {
        fprintf(stderr, "Failed to create AVM\n");
        return 1;
    }
    
    // Create instructions
    Instruction code[] = {
        {OP_LOAD_CONST, 42.0, NULL, 0},
        {OP_PRINT, 0.0, NULL, 0},
        {OP_HLT, 0.0, NULL, 0}
    };
    
    // Empty functions and classes for this simple test
    Function functions[] = {{0}};
    Class classes[] = {{0}};
    
    // Load bytecode into AVM
    if (!avm_load_bytecode(avm, code, sizeof(code)/sizeof(code[0]), 
                          functions, sizeof(functions)/sizeof(functions[0]),
                          classes, sizeof(classes)/sizeof(classes[0]))) {
        fprintf(stderr, "Failed to load bytecode\n");
        avm_destroy(avm);
        return 1;
    }
    
    // Print AVM state for debugging
    avm_print_state(avm);
    
    // Run the AVM
    int result = avm_run(avm);
    printf("AVM execution finished with result: %d\n", result);
    
    // Clean up
    avm_destroy(avm);
    
    return 0;
}
*/