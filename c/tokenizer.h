#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_ARROW,
    TOKEN_NEWLINE,
    TOKEN_EOF,
    TOKEN_AMPERSAND,
    TOKEN_POINTER
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;

typedef struct {
    Token* tokens;
    size_t count;
    size_t capacity;
} Tokens;

Tokens tokenize(const char* source);
void free_tokens(Tokens* tokens);

#endif