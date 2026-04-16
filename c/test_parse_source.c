#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "tokenizer.h"
#include "ast.h"
#include <string.h>

int main() {
    const char* source = "def main() -> int {\n    print(\"Hello from AddX!\")\n    return 0\n}";
    
    printf("Testing parse_source with NULL tokens (should tokenize internally):\n");
    ASTNode* ast1 = parse_source(source, NULL);
    if (ast1) {
        printf("Success! AST type: %d\n", ast1->type);
        // We don't have a nice print function, but at least it didn't crash
    } else {
        printf("Failed to parse\n");
        return 1;
    }
    
    printf("\nTesting parse_source with pre-tokenized tokens:\n");
    Tokens tokens = tokenize(source);
    ASTNode* ast2 = parse_source(source, &tokens);
    if (ast2) {
        printf("Success! AST type: %d\n", ast2->type);
    } else {
        printf("Failed to parse\n");
        free_tokens(&tokens);
        return 1;
    }
    free_tokens(&tokens);
    
    // Also test that we can call parse directly with tokens
    printf("\nTesting parse directly with tokens:\n");
    Tokens tokens2 = tokenize(source);
    ASTNode* ast3 = parse(&tokens2);
    if (ast3) {
        printf("Success! AST type: %d\n", ast3->type);
    } else {
        printf("Failed to parse\n");
        free_tokens(&tokens2);
        return 1;
    }
    free_tokens(&tokens2);
    
    printf("\nAll tests passed!\n");
    return 0;
}