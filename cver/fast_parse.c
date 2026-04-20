#include "fast_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARENA_CHUNK_SIZE (64 * 1024)
#define DEBUG_PARSE 1

#define TRACE(msg) do { printf("[TRACE] %s at pos=%zu char='%c'\n", msg, pos, pos < src_len ? src[pos] : '?'); fflush(stdout); } while(0)

#define TRACE(msg) do { printf("[TRACE] %s at pos=%zu char='%c'\n", msg, pos, src[pos] ? src[pos] : '?'); fflush(stdout); } while(0)

typedef struct StringNode {
    const char* str;
    size_t len;
    uint32_t hash;
    struct StringNode* next;
} StringNode;

static StringNode* string_pool[64] = {0};
static char* string_arena = NULL;
static size_t string_arena_size = 0;
static size_t string_arena_cap = 0;

static uint32_t hash_str(const char* s, size_t len) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        h ^= (uint8_t)s[i];
        h *= 16777619u;
    }
    return h;
}

static const char* intern(const char* s, size_t len) {
    uint32_t h = hash_str(s, len) & 63;
    
    StringNode* n = string_pool[h];
    while (n) {
        if (n->len == len && memcmp(n->str, s, len) == 0) return n->str;
        n = n->next;
    }
    
    if (!string_arena) {
        string_arena = malloc(ARENA_CHUNK_SIZE);
        string_arena_cap = ARENA_CHUNK_SIZE;
    }
    if (string_arena_size + len + 1 > string_arena_cap) {
        string_arena = realloc(string_arena, string_arena_cap * 2);
        string_arena_cap *= 2;
    }
    
    char* dst = string_arena + string_arena_size;
    memcpy(dst, s, len);
    dst[len] = '\0';
    string_arena_size += len + 1;
    
    n = malloc(sizeof(StringNode));
    n->str = dst;
    n->len = len;
    n->hash = h;
    n->next = string_pool[h];
    string_pool[h] = n;
    
    return dst;
}

static ASTNode* alloc_node(ASTNodeType type, void* data) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = type;
    n->data = data;
    return n;
}

static int is_digit(char c) { return c >= '0' && c <= '9'; }
static int is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
static int is_alnum(char c) { return is_alpha(c) || is_digit(c); }

static const char* src;
static size_t src_len;
static size_t pos;
static int current_indent = 0;

static int get_indent() {
    size_t start = pos;
    int spaces = 0;
    while (pos < src_len && (src[pos] == ' ' || src[pos] == '\t')) {
        if (src[pos] == ' ') spaces++;
        else spaces += 4;
        pos++;
    }
    if (pos >= src_len || src[pos] == '\n' || src[pos] == '#') {
        pos = start;
        return -1;
    }
    return spaces;
}

static void skip_ws() {
    while (pos < src_len) {
        char c = src[pos];
        if (c == ' ' || c == '\t') { pos++; continue; }
        if (c == '\n') { 
#if DEBUG_PARSE
            printf("[NEWLINE] pos=%zu\n", pos);
#endif
            pos++; continue; }
        if (c == '#') { while (pos < src_len && src[pos] != '\n') pos++; continue; }
        break;
    }
}

static int match(const char* s) {
    size_t len = strlen(s);
    if (pos + len > src_len) return 0;
    return memcmp(src + pos, s, len) == 0;
}

static int match_kw(const char* s) {
    if (!match(s)) return 0;
    size_t len = strlen(s);
    size_t next = pos + len;
    if (next < src_len && is_alnum(src[next])) return 0;
    return 1;
}

static const char* read_ident() {
    const char* start = src + pos;
    size_t len = 0;
    while (pos < src_len && is_alnum(src[pos])) {
        pos++;
        len++;
    }
    return intern(start, len);
}

static double read_number() {
    const char* start = src + pos;
    size_t len = 0;
    while (pos < src_len && (is_digit(src[pos]) || src[pos] == '.')) {
        pos++;
        len++;
    }
    char buf[64];
    if (len >= 63) len = 62;
    memcpy(buf, start, len);
    buf[len] = '\0';
    return atof(buf);
}

static const char* read_string() {
    char quote = src[pos++];
    const char* start = src + pos;
    size_t len = 0;
    while (pos < src_len && src[pos] != quote) {
        if (src[pos] == '\\' && pos + 1 < src_len) pos++;
        pos++;
        len++;
    }
    if (pos < src_len) pos++;
    return intern(start, len);
}

static ASTNode* parse_exp();

static ASTNode** parse_block(size_t* out_count, int brace);

static ASTNode* parse_function() {
    int is_static = 0;
    
    if (match("@local")) {
        is_static = 1;
        while (pos < src_len && (src[pos] == ' ' || src[pos] == '\t')) pos++;
    }
    
    pos += 3;
    const char* name = read_ident();
    
#if DEBUG_PARSE
    printf("[FUNCTION] name=%s pos=%zu\n", name, pos);
#endif
    
    pos++;
    char** param_names = NULL;
    char** param_types = NULL;
    size_t param_count = 0;
    
    while (pos < src_len && src[pos] != ')') {
        if (param_count > 0) pos++;
        param_names = realloc(param_names, sizeof(char*) * (param_count + 1));
        param_types = realloc(param_types, sizeof(char*) * (param_count + 1));
        param_names[param_count] = (char*)read_ident();
        pos++;
        param_types[param_count] = (char*)read_ident();
        param_count++;
    }
    pos++;
    
    const char* return_type = "void";
    if (match("->")) {
        pos += 2;
        return_type = read_ident();
#if DEBUG_PARSE
        printf("[FUNCTION] return_type=%s\n", return_type);
#endif
    }
    
    pos++;
#if DEBUG_PARSE
    printf("[FUNCTION] body starts at pos=%zu\n", pos);
#endif
    
    size_t body_count = 0;
    ASTNode** body = parse_block(&body_count, 1);
    
#if DEBUG_PARSE
    printf("[FUNCTION] %s body_count=%zu\n", name, body_count);
#endif
    
    FunctionData* d = malloc(sizeof(FunctionData));
    d->name = (char*)name;
    d->param_names = param_names;
    d->param_types = param_types;
    d->param_count = param_count;
    d->return_type = (char*)return_type;
    d->body = body;
    d->body_count = body_count;
    d->is_static = is_static;
    return alloc_node(AST_FUNCTION, d);
}

static ASTNode* parse_if() {
    pos += 2;
#if DEBUG_PARSE
    printf("[IF] condition starts pos=%zu\n", pos);
#endif
    ASTNode* cond = parse_exp();
    
    pos++;
#if DEBUG_PARSE
    printf("[IF] then block starts pos=%zu\n", pos);
#endif
    size_t then_count = 0;
    ASTNode** then_branch = parse_block(&then_count, 1);
    
#if DEBUG_PARSE
    printf("[IF] then_count=%zu, checking for else...\n", then_count);
#endif
    
    ASTNode** else_branch = NULL;
    size_t else_count = 0;
    
    if (match_kw("else")) {
        pos += 4;
        pos++;
#if DEBUG_PARSE
        printf("[IF] else block starts pos=%zu\n", pos);
#endif
        else_branch = parse_block(&else_count, 1);
    }
    
#if DEBUG_PARSE
    printf("[IF] done, else_count=%zu\n", else_count);
#endif
    
    IfData* d = malloc(sizeof(IfData));
    d->condition = cond;
    d->then_branch = then_branch;
    d->then_count = then_count;
    d->else_branch = else_branch;
    d->else_count = else_count;
    return alloc_node(AST_IF, d);
}

static ASTNode* parse_while() {
    pos += 5;
#if DEBUG_PARSE
    printf("[WHILE] condition starts pos=%zu\n", pos);
#endif
    ASTNode* cond = parse_exp();
    
    pos++;
#if DEBUG_PARSE
    printf("[WHILE] body starts pos=%zu\n", pos);
#endif
    size_t body_count = 0;
    ASTNode** body = parse_block(&body_count, 1);
    
#if DEBUG_PARSE
    printf("[WHILE] done, body_count=%zu\n", body_count);
#endif
    
    WhileData* d = malloc(sizeof(WhileData));
    d->condition = cond;
    d->body = body;
    d->body_count = body_count;
    return alloc_node(AST_WHILE, d);
}

static ASTNode* parse_for() {
    pos += 3;
    const char* var_name = read_ident();
    pos += 2;
    
#if DEBUG_PARSE
    printf("[FOR] var=%s pos=%zu\n", var_name, pos);
#endif
    
    ASTNode* start = NULL;
    ASTNode* end = NULL;
    ASTNode* step = NULL;
    
    if (match_kw("range")) {
        pos += 5;
        pos++;
        end = parse_exp();
        if (pos < src_len && src[pos] == ',') {
            pos++;
            start = end;
            end = parse_exp();
            if (pos < src_len && src[pos] == ',') {
                pos++;
                step = parse_exp();
            }
        }
        pos++;
    } else {
        end = parse_exp();
    }
    
    pos++;
#if DEBUG_PARSE
    printf("[FOR] body starts pos=%zu\n", pos);
#endif
    
    size_t body_count = 0;
    ASTNode** body = parse_block(&body_count, 1);
    
#if DEBUG_PARSE
    printf("[FOR] done, body_count=%zu\n", body_count);
#endif
    
    ForData* d = malloc(sizeof(ForData));
    d->var_name = (char*)var_name;
    d->start = start;
    d->end = end;
    d->step = step;
    d->body = body;
    d->body_count = body_count;
    return alloc_node(AST_FOR, d);
}

static ASTNode* parse_return() {
    pos += 6;
#if DEBUG_PARSE
    printf("[RETURN] pos=%zu\n", pos);
#endif
    ASTNode* value = NULL;
    if (pos < src_len && src[pos] != '\n') {
        value = parse_exp();
    }
#if DEBUG_PARSE
    printf("[RETURN] done\n");
#endif
    ReturnData* d = malloc(sizeof(ReturnData));
    d->value = value;
    return alloc_node(AST_RETURN, d);
}

static ASTNode* parse_print() {
    pos += 5;
    pos++;
    
#if DEBUG_PARSE
    printf("[PRINT] args start pos=%zu\n", pos);
#endif
    
    ASTNode** args = NULL;
    size_t arg_count = 0;
    
    while (pos < src_len && src[pos] != ')') {
        if (arg_count > 0) pos++;
        args = realloc(args, sizeof(ASTNode*) * (arg_count + 1));
        args[arg_count++] = parse_exp();
    }
    pos++;
    
#if DEBUG_PARSE
    printf("[PRINT] done, arg_count=%zu\n", arg_count);
#endif
    
    PrintData* d = malloc(sizeof(PrintData));
    d->args = args;
    d->arg_count = arg_count;
    return alloc_node(AST_PRINT, d);
}

static ASTNode* parse_import() {
    pos += 6;
    skip_ws();
    
#if DEBUG_PARSE
    printf("[IMPORT] module name starts pos=%zu\n", pos);
#endif
    
    const char* name = read_ident();
    
#if DEBUG_PARSE
    printf("[IMPORT] module=%s\n", name);
#endif
    
    ImportData* d = malloc(sizeof(ImportData));
    d->module_name = (char*)name;
    return alloc_node(AST_IMPORT, d);
}

static ASTNode* parse_primary() {
    if (pos >= src_len) return NULL;
    char c = src[pos];
    
    if (is_digit(c)) {
#if DEBUG_PARSE
        printf("[NUMBER] pos=%zu\n", pos);
#endif
        NumberData* d = malloc(sizeof(NumberData));
        d->value = read_number();
        return alloc_node(AST_NUMBER, d);
    }
    
    if (c == '"' || c == '\'') {
#if DEBUG_PARSE
        printf("[STRING] pos=%zu\n", pos);
#endif
        StringData* d = malloc(sizeof(StringData));
        d->value = (char*)read_string();
        return alloc_node(AST_STRING, d);
    }
    
    if (c == '(') {
        pos++;
        ASTNode* ex = parse_exp();
        pos++;
        return ex;
    }
    
    if (c == '[') {
        pos++;
        ASTNode** elems = NULL;
        size_t count = 0;
        while (pos < src_len && src[pos] != ']') {
            if (count > 0) pos++;
            elems = realloc(elems, sizeof(ASTNode*) * (count + 1));
            elems[count++] = parse_exp();
        }
        pos++;
        ListData* d = malloc(sizeof(ListData));
        d->elements = elems;
        d->count = count;
        return alloc_node(AST_LIST, d);
    }
    
    if (is_alpha(c)) {
        const char* name = read_ident();
        
#if DEBUG_PARSE
        printf("[IDENTIFIER] name=%s pos=%zu\n", name, pos);
#endif
        
        if (match_kw("True")) {
            BoolData* d = malloc(sizeof(BoolData));
            d->value = 1;
            return alloc_node(AST_BOOL, d);
        }
        if (match_kw("False")) {
            BoolData* d = malloc(sizeof(BoolData));
            d->value = 0;
            return alloc_node(AST_BOOL, d);
        }
        if (match_kw("None")) return alloc_node(AST_NONE, NULL);
        if (match_kw("nullptr")) return alloc_node(AST_NULLPTR, NULL);
        
        if (pos < src_len && src[pos] == ':') {
            pos++;
            const char* vt = read_ident();
#if DEBUG_PARSE
            printf("[VAR_DECL] name=%s type=%s pos=%zu\n", name, vt, pos);
#endif
            VarDeclData* d = malloc(sizeof(VarDeclData));
            d->name = (char*)name;
            d->var_type = (char*)vt;
            d->value = NULL;
            if (pos < src_len && src[pos] == '=') {
                pos++;
                d->value = parse_exp();
            }
            return alloc_node(AST_VAR_DECL, d);
        }
        
        IdentifierData* d = malloc(sizeof(IdentifierData));
        d->name = (char*)name;
        return alloc_node(AST_IDENTIFIER, d);
    }
    
    return NULL;
}

static ASTNode* parse_unary() {
    if (match("-")) {
        pos++;
        UnaryOpData* d = malloc(sizeof(UnaryOpData));
        d->op = "-";
        d->operand = parse_unary();
        return alloc_node(AST_UNARY_OP, d);
    }
    if (match_kw("not")) {
        pos += 3;
        UnaryOpData* d = malloc(sizeof(UnaryOpData));
        d->op = "not";
        d->operand = parse_unary();
        return alloc_node(AST_UNARY_OP, d);
    }
    return parse_primary();
}

static ASTNode* parse_mul() {
    ASTNode* left = parse_unary();
    
    while (pos < src_len) {
        char c = src[pos];
        if (c != '*' && c != '/' && c != '%') break;
        pos++;
        ASTNode* right = parse_unary();
        BinaryOpData* d = malloc(sizeof(BinaryOpData));
        d->op = (c == '*') ? "*" : (c == '/') ? "/" : "%";
        d->left = left;
        d->right = right;
        left = alloc_node(AST_BINARY_OP, d);
    }
    return left;
}

static ASTNode* parse_add() {
    ASTNode* left = parse_mul();
    
    while (pos < src_len) {
        char c = src[pos];
        if (c != '+' && c != '-') break;
        pos++;
        ASTNode* right = parse_mul();
        BinaryOpData* d = malloc(sizeof(BinaryOpData));
        d->op = (c == '+') ? "+" : "-";
        d->left = left;
        d->right = right;
        left = alloc_node(AST_BINARY_OP, d);
    }
    return left;
}

static ASTNode* parse_cmp() {
    ASTNode* left = parse_add();
    
    if (match("==")) { pos += 2; goto bin; }
    if (match("!=")) { pos += 2; goto bin; }
    if (match("<=")) { pos += 2; goto bin; }
    if (match(">=")) { pos += 2; goto bin; }
    if (src[pos] == '<' && pos + 1 < src_len && src[pos+1] != '=') { pos++; goto bin; }
    if (src[pos] == '>' && pos + 1 < src_len && src[pos+1] != '=') { pos++; goto bin; }
    return left;
    
bin: {
        ASTNode* right = parse_add();
        BinaryOpData* d = malloc(sizeof(BinaryOpData));
        d->op = "==";
        d->left = left;
        d->right = right;
        left = alloc_node(AST_BINARY_OP, d);
    }
    return left;
}

static ASTNode* parse_exp() {
    return parse_cmp();
}

static ASTNode** parse_block(size_t* out_count, int brace) {
    int base_indent = 0;
    
    if (brace) {
        pos++;
#if DEBUG_PARSE
        printf("[BLOCK] brace mode, pos=%zu\n", pos);
#endif
    } else {
        base_indent = get_indent();
#if DEBUG_PARSE
        printf("[BLOCK] indent mode, base_indent=%d, pos=%zu\n", base_indent, pos);
#endif
        if (base_indent < 0) base_indent = 0;
    }
    
    ASTNode** body = NULL;
    size_t count = 0;
    size_t cap = 16;
    body = malloc(sizeof(ASTNode*) * cap);
    
    skip_ws();
    
    TRACE("start of block loop");
    
    int loop_count = 0;
    while (pos < src_len) {
        loop_count++;
        if (loop_count > 100) {
            printf("[BLOCK] ERROR: too many iterations, pos=%zu\n", pos);
            break;
        }
        printf("[BLOCK] loop top, pos=%zu, char='%c', count=%d\n", pos, src[pos], loop_count); fflush(stdout);
        if (brace && src[pos] == '}') { 
#if DEBUG_PARSE
            printf("[BLOCK] closing brace at pos=%zu\n", pos);
#endif
            pos++; break; 
        }
        
        if (!brace) {
            while (pos < src_len && (src[pos] == '\n' || src[pos] == ' ' || src[pos] == '\t')) {
                if (src[pos] == '\n') {
#if DEBUG_PARSE
                    printf("[BLOCK] skipping newline at pos=%zu\n", pos);
#endif
                }
                pos++;
            }
            if (pos >= src_len) break;
            
            int line_indent = get_indent();
            if (line_indent < 0) {
                line_indent = 0;
                pos--;
            }
#if DEBUG_PARSE
            printf("[BLOCK] line indent=%d, base=%d, char='%c'\n", line_indent, base_indent, src[pos]);
#endif
            if (line_indent <= base_indent && src[pos] != '\0') {
#if DEBUG_PARSE
                printf("[BLOCK] dedent detected, stopping\n");
#endif
                break;
            }
        }
        
        if (match_kw("def")) { ASTNode* f = parse_function(); body[count++] = f; skip_ws(); continue; }
        if (match_kw("if")) { ASTNode* f = parse_if(); body[count++] = f; skip_ws(); continue; }
        if (match_kw("while")) { ASTNode* f = parse_while(); body[count++] = f; skip_ws(); continue; }
        if (match_kw("for")) { ASTNode* f = parse_for(); body[count++] = f; skip_ws(); continue; }
        if (match_kw("return")) { ASTNode* f = parse_return(); body[count++] = f; skip_ws(); continue; }
        if (match_kw("print")) { ASTNode* f = parse_print(); body[count++] = f; skip_ws(); continue; }
        if (match_kw("import")) { ASTNode* f = parse_import(); body[count++] = f; skip_ws(); continue; }
        
        ASTNode* st = parse_exp();
        if (st) {
            if (count >= cap) { cap *= 2; body = realloc(body, sizeof(ASTNode*) * cap); }
            body[count++] = st;
        }
        
        while (pos < src_len && (src[pos] == '\n' || src[pos] == ' ' || src[pos] == '\t')) {
            if (src[pos] == '\n') {
#if DEBUG_PARSE
                printf("[BLOCK] statement end newline at pos=%zu\n", pos);
#endif
            }
            pos++;
        }
    }
    
    *out_count = count;
    return body;
}

ASTNode* fast_parse(const char* source) {
    printf("[fast_parse] START, src_len=%zu\n", strlen(source)); fflush(stdout);
    src = source;
    src_len = strlen(source);
    pos = 0;
    
    size_t body_count = 0;
    printf("[fast_parse] calling parse_block\n"); fflush(stdout);
    ASTNode** body = parse_block(&body_count, 1);  // Use brace mode for now
    
    printf("[fast_parse] END, body_count=%zu\n", body_count); fflush(stdout);
    ProgramData* data = malloc(sizeof(ProgramData));
    data->statements = body;
    data->count = body_count;
    
    return alloc_node(AST_PROGRAM, data);
}

ASTNode* fast_parse_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);
    
    ASTNode* ast = fast_parse(source);
    free(source);
    
    return ast;
}

void fast_free(ASTNode* ast) {
    (void)ast;
}