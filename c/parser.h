#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include "ast.h"

typedef struct {
    Tokens* tokens;
    size_t pos;
} Parser;

ASTNode* parse(Tokens* tokens);
void skip_newlines(Parser* p);
ASTNode* parse_source(const char* source, Tokens* tokens);

#endif