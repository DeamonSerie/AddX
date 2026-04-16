#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "parser.h"

#define PARSER_DYNAMIC_ARRAY 1
#define PARSER_HASH_LOOKUP 1
#define PARSER_LOCAL_CACHE 1

#if PARSER_LOCAL_CACHE
#define CACHE_LOCAL(vars, p) vars = p
#endif

static Token peek(Parser* p) {
    if (p->pos < p->tokens->count) return p->tokens->tokens[p->pos];
    static Token eof = {TOKEN_EOF, NULL, 0, 0};
    return eof;
}

static Token consume(Parser* p) {
    if (p->pos < p->tokens->count) return p->tokens->tokens[p->pos++];
    static Token eof = {TOKEN_EOF, NULL, 0, 0};
    return eof;
}

static int check(Parser* p, TokenType type, const char* value) {
    if (p->pos >= p->tokens->count) return 0;
    Token* t = &p->tokens->tokens[p->pos];
    if (t->type != type) return 0;
    if (value && (!t->value || strcmp(t->value, value) != 0)) return 0;
    return 1;
}

static int check_type(Parser* p, TokenType type) {
    if (p->pos >= p->tokens->count) return 0;
    return p->tokens->tokens[p->pos].type == type;
}

static int check_keyword(Parser* p, const char* kw) {
    if (p->pos >= p->tokens->count) return 0;
    Token* t = &p->tokens->tokens[p->pos];
    if (t->type != TOKEN_KEYWORD || !t->value) return 0;
    return strcmp(t->value, kw) == 0;
}

static Token expect(Parser* p, TokenType type, const char* value) {
    if (p->pos >= p->tokens->count) {
        fprintf(stderr, "Unexpected EOF\n");
        exit(1);
    }
    Token* t = &p->tokens->tokens[p->pos];
    if (t->type != type || (value && (!t->value || strcmp(t->value, value) != 0))) {
        fprintf(stderr, "Expected %d %s, got %d\n", type, value ? value : "", t->type);
        exit(1);
    }
    return consume(p);
}

void skip_newlines(Parser* p) {
    while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_NEWLINE) {
        p->pos++;
    }
}

static ASTNode* parse_stmt(Parser* p);
static ASTNode* parse_exp(Parser* p);

static ASTNode** grow_array(ASTNode** arr, size_t* cap, size_t* count) {
    if (*count >= *cap) {
        *cap = *cap ? *cap * 2 : 16;
        arr = realloc(arr, sizeof(ASTNode*) * *cap);
    }
    return arr;
}

static ASTNode* parse_program(Parser* p) {
    ASTNode** stmts = NULL;
    size_t count = 0;
    size_t capacity = 0;
    
    while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type != TOKEN_EOF) {
        skip_newlines(p);
        if (p->pos >= p->tokens->count) break;
        ASTNode* st = parse_stmt(p);
        if (st) {
            stmts = grow_array(stmts, &capacity, &count);
            stmts[count++] = st;
        }
    }
    
    ProgramData* data = malloc(sizeof(ProgramData));
    data->statements = stmts;
    data->count = count;
    return create_node(AST_PROGRAM, data);
}

static ASTNode** parse_block(Parser* p, size_t* out_count) {
    ASTNode** body = NULL;
    size_t count = 0;
    size_t capacity = 0;
    int depth = 0;
    
    while (p->pos < p->tokens->count) {
        skip_newlines(p);
        Token* t = &p->tokens->tokens[p->pos];
        
        if (t->type == TOKEN_EOF) break;
        if (t->type == TOKEN_LBRACE) {
            p->pos++;
            depth++;
            continue;
        }
        if (t->type == TOKEN_RBRACE) {
            if (depth == 0) {
                p->pos++;
                break;
            }
            p->pos++;
            depth--;
            continue;
        }
        if (t->type == TOKEN_KEYWORD && depth == 0 && (
            strcmp(t->value, "def") == 0 || strcmp(t->value, "class") == 0)) {
            break;
        }
        
        ASTNode* st = parse_stmt(p);
        if (st) {
            body = grow_array(body, &capacity, &count);
            body[count++] = st;
        }
    }
    
    *out_count = count;
    return body;
}

static ASTNode* parse_function(Parser* p) {
    p->pos++;
    
    Token* name_tok = &p->tokens->tokens[p->pos];
    char* name = strdup(name_tok->value);
    p->pos++;
    
    p->pos++;
    
    char** param_names = NULL;
    char** param_types = NULL;
    size_t param_count = 0;
    
    while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type != TOKEN_RPAREN) {
        if (param_count > 0 && p->tokens->tokens[p->pos].type == TOKEN_COMMA) p->pos++;
        
        Token* pn = &p->tokens->tokens[p->pos];
        char* pn_str = strdup(pn->value);
        p->pos++;
        p->pos++;
        Token* pt = &p->tokens->tokens[p->pos];
        char* pt_str = strdup(pt->value);
        p->pos++;
        
        param_names = realloc(param_names, sizeof(char*) * (param_count + 1));
        param_types = realloc(param_types, sizeof(char*) * (param_count + 1));
        param_names[param_count] = pn_str;
        param_types[param_count] = pt_str;
        param_count++;
    }
    p->pos++;
    
    char* return_type = "void";
    if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_ARROW) {
        p->pos++;
        return_type = strdup(p->tokens->tokens[p->pos].value);
        p->pos++;
    }
    
    p->pos++;
    
    size_t body_count = 0;
    ASTNode** body = parse_block(p, &body_count);
    
    FunctionData* data = malloc(sizeof(FunctionData));
    data->name = name;
    data->param_names = param_names;
    data->param_types = param_types;
    data->param_count = param_count;
    data->return_type = return_type;
    data->body = body;
    data->body_count = body_count;
    return create_node(AST_FUNCTION, data);
}

static ASTNode* parse_if(Parser* p) {
    p->pos++;
    ASTNode* cond = parse_exp(p);
    
    p->pos++;
    size_t then_count = 0;
    ASTNode** then_branch = parse_block(p, &then_count);
    
    ASTNode** else_branch = NULL;
    size_t else_count = 0;
    
    if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_KEYWORD && 
        strcmp(p->tokens->tokens[p->pos].value, "else") == 0) {
        p->pos++;
        p->pos++;
        else_branch = parse_block(p, &else_count);
    }
    
    IfData* data = malloc(sizeof(IfData));
    data->condition = cond;
    data->then_branch = then_branch;
    data->then_count = then_count;
    data->else_branch = else_branch;
    data->else_count = else_count;
    return create_node(AST_IF, data);
}

static ASTNode* parse_while(Parser* p) {
    p->pos++;
    ASTNode* cond = parse_exp(p);
    
    p->pos++;
    size_t body_count = 0;
    ASTNode** body = parse_block(p, &body_count);
    
    WhileData* data = malloc(sizeof(WhileData));
    data->condition = cond;
    data->body = body;
    data->body_count = body_count;
    return create_node(AST_WHILE, data);
}

static ASTNode* parse_for(Parser* p) {
    p->pos++;
    Token* var_tok = &p->tokens->tokens[p->pos];
    char* var_name = strdup(var_tok->value);
    p->pos++;
    
    if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_KEYWORD && 
        strcmp(p->tokens->tokens[p->pos].value, "in") == 0) p->pos++;
    
    ASTNode* start = NULL;
    ASTNode* end = NULL;
    ASTNode* step = NULL;
    
    if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_KEYWORD && 
        strcmp(p->tokens->tokens[p->pos].value, "range") == 0) {
        p->pos++;
        p->pos++;
        
        end = parse_exp(p);
        if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_COMMA) {
            p->pos++;
            start = end;
            end = parse_exp(p);
            if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_COMMA) {
                p->pos++;
                step = parse_exp(p);
            }
        }
        p->pos++;
    } else {
        end = parse_exp(p);
    }
    
    p->pos++;
    size_t body_count = 0;
    ASTNode** body = parse_block(p, &body_count);
    
    ForData* data = malloc(sizeof(ForData));
    data->var_name = var_name;
    data->start = start;
    data->end = end;
    data->step = step;
    data->body = body;
    data->body_count = body_count;
    return create_node(AST_FOR, data);
}

static ASTNode* parse_return(Parser* p) {
    p->pos++;
    ASTNode* value = NULL;
    if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type != TOKEN_NEWLINE && 
        p->tokens->tokens[p->pos].type != TOKEN_EOF) {
        value = parse_exp(p);
    }
    ReturnData* data = malloc(sizeof(ReturnData));
    data->value = value;
    return create_node(AST_RETURN, data);
}

static ASTNode* parse_print(Parser* p) {
    p->pos++;
    p->pos++;
    
    ASTNode** args = NULL;
    size_t arg_count = 0;
    
    while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type != TOKEN_RPAREN) {
        if (arg_count > 0 && p->tokens->tokens[p->pos].type == TOKEN_COMMA) p->pos++;
        args = realloc(args, sizeof(ASTNode*) * (arg_count + 1));
        args[arg_count++] = parse_exp(p);
    }
    p->pos++;
    
    PrintData* data = malloc(sizeof(PrintData));
    data->args = args;
    data->arg_count = arg_count;
    return create_node(AST_PRINT, data);
}

static ASTNode* parse_static(Parser* p) {
    p->pos++;
    Token* name = &p->tokens->tokens[p->pos];
    char* name_str = strdup(name->value);
    p->pos += 2;
    Token* vt = &p->tokens->tokens[p->pos];
    char* vt_str = strdup(vt->value);
    p->pos++;
    
    ASTNode* value = NULL;
    if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_OPERATOR && 
        strcmp(p->tokens->tokens[p->pos].value, "=") == 0) {
        p->pos++;
        value = parse_exp(p);
    }
    
    StaticVarData* data = malloc(sizeof(StaticVarData));
    data->name = name_str;
    data->var_type = vt_str;
    data->value = value;
    return create_node(AST_STATIC_VAR, data);
}

static ASTNode* parse_const(Parser* p) {
    p->pos++;
    Token* name = &p->tokens->tokens[p->pos];
    char* name_str = strdup(name->value);
    p->pos += 2;
    Token* vt = &p->tokens->tokens[p->pos];
    char* vt_str = strdup(vt->value);
    p->pos++;
    p->pos++;
    ASTNode* value = parse_exp(p);
    
    ConstData* data = malloc(sizeof(ConstData));
    data->name = name_str;
    data->var_type = vt_str;
    data->value = value;
    return create_node(AST_CONST, data);
}

static ASTNode* parse_class(Parser* p) {
    p->pos++;
    Token* name = &p->tokens->tokens[p->pos];
    char* name_str = strdup(name->value);
    p->pos++;
    
    p->pos++;
    size_t body_count = 0;
    ASTNode** body = parse_block(p, &body_count);
    
    ClassData* data = malloc(sizeof(ClassData));
    data->name = name_str;
    data->base_class = NULL;
    data->body = body;
    data->body_count = body_count;
    return create_node(AST_CLASS, data);
}

static ASTNode* parse_primary(Parser* p) {
    Token* t = &p->tokens->tokens[p->pos];
    
    if (t->type == TOKEN_NUMBER) {
        p->pos++;
        NumberData* data = malloc(sizeof(NumberData));
        data->value = atof(t->value);
        return create_node(AST_NUMBER, data);
    }
    
    if (t->type == TOKEN_STRING) {
        p->pos++;
        StringData* data = malloc(sizeof(StringData));
        data->value = strdup(t->value);
        return create_node(AST_STRING, data);
    }
    
    if (t->type == TOKEN_KEYWORD) {
        if (strcmp(t->value, "True") == 0) { p->pos++; BoolData* d = malloc(sizeof(BoolData)); d->value = 1; return create_node(AST_BOOL, d); }
        if (strcmp(t->value, "False") == 0) { p->pos++; BoolData* d = malloc(sizeof(BoolData)); d->value = 0; return create_node(AST_BOOL, d); }
        if (strcmp(t->value, "None") == 0) { p->pos++; return create_node(AST_NONE, NULL); }
        if (strcmp(t->value, "nullptr") == 0) { p->pos++; return create_node(AST_NULLPTR, NULL); }
        
        if (strcmp(t->value, "sizeof") == 0) {
            p->pos++; p->pos++;
            char* type_name = NULL;
            ASTNode** expr = NULL;
            size_t expr_count = 0;
            if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_KEYWORD) {
                type_name = strdup(p->tokens->tokens[p->pos].value);
                p->pos++;
            } else {
                expr = malloc(sizeof(ASTNode*));
                expr[0] = parse_exp(p);
                expr_count = 1;
            }
            p->pos++;
            SizeofData* data = malloc(sizeof(SizeofData));
            data->type_name = type_name;
            data->expr = expr;
            data->expr_count = expr_count;
            return create_node(AST_SIZEOF, data);
        }
        
        if (strcmp(t->value, "new") == 0) {
            p->pos++;
            char* type_name = strdup(p->tokens->tokens[p->pos].value);
            p->pos++;
            ASTNode** args = NULL;
            size_t arg_count = 0;
            if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_LPAREN) {
                p->pos++;
                while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type != TOKEN_RPAREN) {
                    if (arg_count > 0 && p->tokens->tokens[p->pos].type == TOKEN_COMMA) p->pos++;
                    args = realloc(args, sizeof(ASTNode*) * (arg_count + 1));
                    args[arg_count++] = parse_exp(p);
                }
                p->pos++;
            }
            NewData* data = malloc(sizeof(NewData));
            data->type_name = type_name;
            data->args = args;
            data->arg_count = arg_count;
            return create_node(AST_NEW, data);
        }
        
        if (strcmp(t->value, "delete") == 0) {
            p->pos++;
            ASTNode* expr = parse_exp(p);
            DeleteData* data = malloc(sizeof(DeleteData));
            data->expr = expr;
            return create_node(AST_DELETE, data);
        }
    }
    
    if (t->type == TOKEN_IDENTIFIER) {
        char* name = strdup(t->value);
        p->pos++;
        
        if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_COLON) {
            p->pos++;
            char* vt = strdup(p->tokens->tokens[p->pos].value);
            p->pos++;
            VarDeclData* data = malloc(sizeof(VarDeclData));
            data->name = name;
            data->var_type = vt;
            data->value = NULL;
            if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_OPERATOR && 
                strcmp(p->tokens->tokens[p->pos].value, "=") == 0) {
                p->pos++;
                data->value = parse_exp(p);
            }
            return create_node(AST_VAR_DECL, data);
        }
        
        IdentifierData* data = malloc(sizeof(IdentifierData));
        data->name = name;
        return create_node(AST_IDENTIFIER, data);
    }
    
    if (t->type == TOKEN_LBRACKET) {
        p->pos++;
        ASTNode** elems = NULL;
        size_t count = 0;
        while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type != TOKEN_RBRACKET) {
            if (count > 0 && p->tokens->tokens[p->pos].type == TOKEN_COMMA) p->pos++;
            elems = realloc(elems, sizeof(ASTNode*) * (count + 1));
            elems[count++] = parse_exp(p);
        }
        p->pos++;
        ListData* data = malloc(sizeof(ListData));
        data->elements = elems;
        data->count = count;
        return create_node(AST_LIST, data);
    }
    
    if (t->type == TOKEN_LPAREN) {
        p->pos++;
        ASTNode* ex = parse_exp(p);
        p->pos++;
        return ex;
    }
    
    if (t->type == TOKEN_AMPERSAND) {
        p->pos++;
        char* var_name = strdup(p->tokens->tokens[p->pos].value);
        p->pos++;
        IdentifierData* id = malloc(sizeof(IdentifierData));
        id->name = var_name;
        return create_node(AST_ADDRESS_OF, id);
    }
    
    if (t->type == TOKEN_POINTER) {
        p->pos++;
        ASTNode* ptr = parse_primary(p);
        return create_node(AST_DEREFERENCE, ptr);
    }
    
    fprintf(stderr, "Bad token: %s\n", t->value ? t->value : "<unknown>");
    exit(1);
}

static ASTNode* parse_unary(Parser* p) {
    if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_OPERATOR && 
        strcmp(p->tokens->tokens[p->pos].value, "-") == 0) {
        p->pos++;
        ASTNode* operand = parse_unary(p);
        UnaryOpData* data = malloc(sizeof(UnaryOpData));
        data->op = strdup("-");
        data->operand = operand;
        return create_node(AST_UNARY_OP, data);
    }
    if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_KEYWORD && 
        strcmp(p->tokens->tokens[p->pos].value, "not") == 0) {
        p->pos++;
        ASTNode* operand = parse_unary(p);
        UnaryOpData* data = malloc(sizeof(UnaryOpData));
        data->op = strdup("not");
        data->operand = operand;
        return create_node(AST_UNARY_OP, data);
    }
    return parse_primary(p);
}

static ASTNode* parse_mul(Parser* p) {
    ASTNode* left = parse_unary(p);
    
    while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_OPERATOR) {
        Token* t = &p->tokens->tokens[p->pos];
        if (strcmp(t->value, "*") != 0 && strcmp(t->value, "/") != 0 && strcmp(t->value, "%") != 0) break;
        p->pos++;
        ASTNode* right = parse_unary(p);
        BinaryOpData* data = malloc(sizeof(BinaryOpData));
        data->op = strdup(t->value);
        data->left = left;
        data->right = right;
        left = create_node(AST_BINARY_OP, data);
    }
    return left;
}

static ASTNode* parse_add(Parser* p) {
    ASTNode* left = parse_mul(p);
    
    while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_OPERATOR) {
        Token* t = &p->tokens->tokens[p->pos];
        if (strcmp(t->value, "+") != 0 && strcmp(t->value, "-") != 0) break;
        p->pos++;
        ASTNode* right = parse_mul(p);
        BinaryOpData* data = malloc(sizeof(BinaryOpData));
        data->op = strdup(t->value);
        data->left = left;
        data->right = right;
        left = create_node(AST_BINARY_OP, data);
    }
    return left;
}

static ASTNode* parse_cmp(Parser* p) {
    ASTNode* left = parse_add(p);
    
    while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_OPERATOR) {
        Token* t = &p->tokens->tokens[p->pos];
        if (strcmp(t->value, "==") != 0 && strcmp(t->value, "!=") != 0 &&
            strcmp(t->value, "<") != 0 && strcmp(t->value, ">") != 0 &&
            strcmp(t->value, "<=") != 0 && strcmp(t->value, ">=") != 0) break;
        p->pos++;
        ASTNode* right = parse_add(p);
        BinaryOpData* data = malloc(sizeof(BinaryOpData));
        data->op = strdup(t->value);
        data->left = left;
        data->right = right;
        left = create_node(AST_BINARY_OP, data);
    }
    return left;
}

static ASTNode* parse_and(Parser* p) {
    ASTNode* left = parse_cmp(p);
    
    while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_KEYWORD && 
           strcmp(p->tokens->tokens[p->pos].value, "and") == 0) {
        p->pos++;
        ASTNode* right = parse_cmp(p);
        BinaryOpData* data = malloc(sizeof(BinaryOpData));
        data->op = strdup("and");
        data->left = left;
        data->right = right;
        left = create_node(AST_BINARY_OP, data);
    }
    return left;
}

static ASTNode* parse_or(Parser* p) {
    ASTNode* left = parse_and(p);
    
    while (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_KEYWORD && 
           strcmp(p->tokens->tokens[p->pos].value, "or") == 0) {
        p->pos++;
        ASTNode* right = parse_and(p);
        BinaryOpData* data = malloc(sizeof(BinaryOpData));
        data->op = strdup("or");
        data->left = left;
        data->right = right;
        left = create_node(AST_BINARY_OP, data);
    }
    return left;
}

static ASTNode* parse_exp(Parser* p) {
    return parse_or(p);
}

static ASTNode* parse_expr_stmt(Parser* p) {
    ASTNode* e = parse_exp(p);
    
    if (p->pos < p->tokens->count && p->tokens->tokens[p->pos].type == TOKEN_OPERATOR && 
        strcmp(p->tokens->tokens[p->pos].value, "=") == 0) {
        p->pos++;
        ASTNode* val = parse_exp(p);
        if (e->type == AST_IDENTIFIER) {
            IdentifierData* id = e->data;
            AssignmentData* data = malloc(sizeof(AssignmentData));
            data->target = strdup(id->name);
            data->value = val;
            free(e->data);
            e->data = data;
            e->type = AST_ASSIGNMENT;
            return e;
        }
    }
    return e;
}

static ASTNode* parse_stmt(Parser* p) {
    if (p->pos >= p->tokens->count || p->tokens->tokens[p->pos].type == TOKEN_EOF) return NULL;
    if (p->tokens->tokens[p->pos].type == TOKEN_RBRACE) return NULL;
    
    if (p->tokens->tokens[p->pos].type == TOKEN_KEYWORD) {
        Token* t = &p->tokens->tokens[p->pos];
        
        if (strcmp(t->value, "def") == 0) return parse_function(p);
        if (strcmp(t->value, "if") == 0) return parse_if(p);
        if (strcmp(t->value, "for") == 0) return parse_for(p);
        if (strcmp(t->value, "while") == 0) return parse_while(p);
        if (strcmp(t->value, "return") == 0) return parse_return(p);
        if (strcmp(t->value, "print") == 0) return parse_print(p);
        if (strcmp(t->value, "static") == 0) return parse_static(p);
        if (strcmp(t->value, "const") == 0) return parse_const(p);
        if (strcmp(t->value, "class") == 0) return parse_class(p);
    }
    
    return parse_expr_stmt(p);
}

ASTNode* parse(Tokens* tokens) {
    Parser p;
    p.tokens = tokens;
    p.pos = 0;
    return parse_program(&p);
}

ASTNode* parse_source(const char* source, Tokens* tokens) {
    int own_tokens = 0;
    if (tokens == NULL) {
        Tokens local_tokens = tokenize(source);
        tokens = &local_tokens;
        own_tokens = 1;
    }
    ASTNode* result = parse(tokens);
    if (own_tokens) {
        free_tokens(tokens);
    }
    return result;
}