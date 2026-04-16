#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "tokenizer.h"

#define TOKENIZER_OPT 1

#if TOKENIZER_OPT
#define KEYWORD_HASH 1
#define BUFFER_SCAN 1
#endif

#if KEYWORD_HASH
#define KW_HASH_SIZE 32

static int keyword_hash(const char* key) {
    int h = 0;
    while (*key) h = (h * 31 + *key++) % KW_HASH_SIZE;
    return h;
}

typedef struct {
    const char* kw;
    int hash;
} KeywordEntry;

static KeywordEntry KEYWORDS[] = {
    {"def", 0}, {"if", 0}, {"elif", 0}, {"else", 0}, {"for", 0}, {"while", 0}, {"return", 0}, {"print", 0},
    {"class", 0}, {"True", 0}, {"False", 0}, {"None", 0}, {"and", 0}, {"or", 0}, {"not", 0}, {"in", 0},
    {"range", 0}, {"nullptr", 0}, {"sizeof", 0}, {"static", 0}, {"const", 0}, {"new", 0}, {"delete", 0},
    {"int", 0}, {"float", 0}, {"str", 0}, {"bool", 0}, {"void", 0}, {"list", 0}, {"dict", 0}, {"auto", 0}
};

static int kw_lookup[KW_HASH_SIZE];
static int kw_initialized = 0;

static void init_kw() {
    if (kw_initialized) return;
    for (int i = 0; i < KW_HASH_SIZE; i++) kw_lookup[i] = -1;
    for (int i = 0; i < 36; i++) {
        KEYWORDS[i].hash = keyword_hash(KEYWORDS[i].kw);
        int h = KEYWORDS[i].hash;
        kw_lookup[h] = i;
    }
    kw_initialized = 1;
}

static int is_keyword(const char* value) {
    init_kw();
    int h = keyword_hash(value);
    int idx = kw_lookup[h];
    if (idx >= 0 && strcmp(KEYWORDS[idx].kw, value) == 0) return 1;
    return 0;
}

static int is_type_keyword(const char* value) {
    return is_keyword(value);
}
#else
static const char* KEYWORDS[] = {
    "def", "if", "elif", "else", "for", "while", "return", "print",
    "class", "True", "False", "None", "and", "or", "not", "in", "range",
    "nullptr", "sizeof", "static", "const", "new", "delete", NULL
};

static const char* TYPE_KEYWORDS[] = {
    "int", "float", "str", "bool", "void", "list", "dict", "auto", NULL
};

static int is_keyword(const char* value) {
    for (int i = 0; KEYWORDS[i] != NULL; i++) {
        if (strcmp(value, KEYWORDS[i]) == 0) return 1;
    }
    return 0;
}

static int is_type_keyword(const char* value) {
    for (int i = 0; TYPE_KEYWORDS[i] != NULL; i++) {
        if (strcmp(value, TYPE_KEYWORDS[i]) == 0) return 1;
    }
    return 0;
}
#endif

static char* copy_string(const char* start, size_t len) {
    char* result = malloc(len + 1);
    strncpy(result, start, len);
    result[len] = '\0';
    return result;
}

static void add_token(Tokens* tokens, Token token) {
    if (tokens->count >= tokens->capacity) {
        tokens->capacity = tokens->capacity ? tokens->capacity * 2 : 64;
        tokens->tokens = realloc(tokens->tokens, sizeof(Token) * tokens->capacity);
    }
    tokens->tokens[tokens->count++] = token;
}

#if BUFFER_SCAN
typedef struct {
    const char* start;
    const char* end;
    const char* p;
    int line;
    int col;
} ScanState;

static Token scan_number(ScanState* s) {
    const char* start = s->p;
    while (s->p < s->end && (*s->p == '.' || isdigit(*s->p))) s->p++;
    int col = (int)(start - s->start) + 1;
    return (Token){TOKEN_NUMBER, copy_string(start, s->p - start), s->line, col};
}

static Token scan_string(ScanState* s) {
    char quote = *s->p++;
    const char* start = s->p;
    while (s->p < s->end && *s->p != quote) {
        if (*s->p == '\\' && s->p + 1 < s->end) s->p++;
        s->p++;
    }
    char* val = copy_string(start, s->p - start);
    if (s->p < s->end) s->p++;
    int col = (int)(start - s->start);
    return (Token){TOKEN_STRING, val, s->line, col};
}

static Token scan_identifier(ScanState* s) {
    const char* start = s->p;
    while (s->p < s->end && (isalnum(*s->p) || *s->p == '_')) s->p++;
    char* val = copy_string(start, s->p - start);
    TokenType type = is_keyword(val) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    int col = (int)(start - s->start) + 1;
    return (Token){type, val, s->line, col};
}
#endif

Tokens tokenize(const char* source) {
    Tokens tokens = {0};
    
#if BUFFER_SCAN
    ScanState s = {source, source + strlen(source), source, 1, 1};
    
    while (s.p < s.end) {
        if (*s.p == '\n' || *s.p == '\r') {
            add_token(&tokens, (Token){TOKEN_NEWLINE, NULL, s.line, (int)(s.p - s.start) + 1});
            if (*s.p == '\r' && s.p + 1 < s.end && s.p[1] == '\n') s.p++;
            s.p++;
            s.line++;
            s.col = 1;
            continue;
        }
        
        if (*s.p == ' ' || *s.p == '\t') { s.p++; s.col++; continue; }
        if (*s.p == '#') { while (s.p < s.end && *s.p != '\n') s.p++; continue; }
        
        int col = (int)(s.p - s.start) + 1;
        
        if (isdigit(*s.p)) {
            add_token(&tokens, scan_number(&s));
            continue;
        }
        
        if (*s.p == '"' || *s.p == '\'') {
            add_token(&tokens, scan_string(&s));
            continue;
        }
        
        if (isalpha(*s.p) || *s.p == '_') {
            add_token(&tokens, scan_identifier(&s));
            continue;
        }
        
        TokenType t = TOKEN_OPERATOR;
        switch (*s.p) {
            case '(': t = TOKEN_LPAREN; break;
            case ')': t = TOKEN_RPAREN; break;
            case '{': t = TOKEN_LBRACE; break;
            case '}': t = TOKEN_RBRACE; break;
            case '[': t = TOKEN_LBRACKET; break;
            case ']': t = TOKEN_RBRACKET; break;
            case ':': t = TOKEN_COLON; break;
            case ',': t = TOKEN_COMMA; break;
            case '&': t = TOKEN_AMPERSAND; break;
            case '*': t = TOKEN_POINTER; break;
        }
        
        if (s.p + 1 < s.end && *s.p == '-' && s.p[1] == '>') {
            t = TOKEN_ARROW;
            add_token(&tokens, (Token){t, NULL, s.line, col});
            s.p += 2;
            continue;
        }
        
        if (strchr("+-*/%=<>!", *s.p)) {
            const char* start = s.p;
            while (s.p < s.end && strchr("+-*/%=<>!", *s.p)) s.p++;
            add_token(&tokens, (Token){TOKEN_OPERATOR, copy_string(start, s.p - start), s.line, col});
            continue;
        }
        
        add_token(&tokens, (Token){t, NULL, s.line, col});
        s.p++;
    }
    
    add_token(&tokens, (Token){TOKEN_EOF, NULL, s.line, 1});
#else
    const char* lines = source;
    int line_num = 1;
    
    while (*lines) {
        const char* line = lines;
        while (*line == ' ' || *line == '\t') line++;
        
        if (!*line || *line == '#') {
            Token t = {TOKEN_NEWLINE, NULL, line_num, 1};
            add_token(&tokens, t);
            lines = strchr(lines, '\n');
            if (!lines) break;
            lines++;
            line_num++;
            continue;
        }
        
        int col = 1;
        const char* p = line;
        
        while (*p) {
            col = (int)(p - line) + 1;
            
            if (isdigit(*p)) {
                const char* start = p;
                while (isdigit(*p) || *p == '.') p++;
                Token t = {TOKEN_NUMBER, copy_string(start, p - start), line_num, col};
                add_token(&tokens, t);
                continue;
            }
            
            if (*p == '"' || *p == '\'') {
                char quote = *p;
                const char* start = ++p;
                while (*p && *p != quote) {
                    if (*p == '\\' && p[1]) p++;
                    p++;
                }
                char* val = copy_string(start, p - start);
                if (*p) p++;
                Token t = {TOKEN_STRING, val, line_num, col};
                add_token(&tokens, t);
                continue;
            }
            
            if (isalpha(*p) || *p == '_') {
                const char* start = p;
                while (isalnum(*p) || *p == '_') p++;
                char* val = copy_string(start, p - start);
                TokenType type = is_keyword(val) || is_type_keyword(val) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
                Token t = {type, val, line_num, col};
                add_token(&tokens, t);
                continue;
            }
            
            if (*p == '#') break;
            
            if (*p == '(') { Token t = {TOKEN_LPAREN, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            if (*p == ')') { Token t = {TOKEN_RPAREN, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            if (*p == '{') { Token t = {TOKEN_LBRACE, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            if (*p == '}') { Token t = {TOKEN_RBRACE, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            if (*p == '[') { Token t = {TOKEN_LBRACKET, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            if (*p == ']') { Token t = {TOKEN_RBRACKET, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            if (*p == ':') { Token t = {TOKEN_COLON, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            if (*p == ',') { Token t = {TOKEN_COMMA, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            
            if (*p == '-' && p[1] == '>') {
                Token t = {TOKEN_ARROW, NULL, line_num, col};
                add_token(&tokens, t);
                p += 2;
                continue;
            }
            
            if (*p == '&') { Token t = {TOKEN_AMPERSAND, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            if (*p == '*') { Token t = {TOKEN_POINTER, NULL, line_num, col}; add_token(&tokens, t); p++; continue; }
            
            if (strchr("+-*/%=<>!", *p)) {
                const char* start = p;
                while (strchr("+-*/%=<>!", *p)) p++;
                Token t = {TOKEN_OPERATOR, copy_string(start, p - start), line_num, col};
                add_token(&tokens, t);
                continue;
            }
            
            p++;
        }
        
        Token t = {TOKEN_NEWLINE, NULL, line_num, (int)strlen(line) + 1};
        add_token(&tokens, t);
        
        lines = strchr(lines, '\n');
        if (!lines) break;
        lines++;
        line_num++;
    }
    
    Token eof = {TOKEN_EOF, NULL, line_num, 1};
    add_token(&tokens, eof);
#endif
    
    return tokens;
}

void free_tokens(Tokens* tokens) {
    for (size_t i = 0; i < tokens->count; i++) {
        free(tokens->tokens[i].value);
    }
    free(tokens->tokens);
    tokens->tokens = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}