#ifndef FAST_PARSE_H
#define FAST_PARSE_H

#include <stddef.h>

typedef enum {
    AST_PROGRAM, AST_FUNCTION, AST_CLASS, AST_VAR_DECL, AST_ASSIGNMENT,
    AST_IF, AST_WHILE, AST_FOR, AST_RETURN, AST_PRINT,
    AST_BINARY_OP, AST_UNARY_OP, AST_NUMBER, AST_STRING, AST_BOOL,
    AST_NONE, AST_IDENTIFIER, AST_LIST, AST_INDEX_ACCESS,
    AST_ATTRIBUTE_ACCESS, AST_CALL, AST_BREAK, AST_CONTINUE,
    AST_NULLPTR, AST_ADDRESS_OF, AST_DEREFERENCE, AST_SIZEOF,
    AST_STATIC_VAR, AST_CONST, AST_NEW, AST_DELETE, AST_IMPORT
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    void* data;
} ASTNode;

typedef struct {
    ASTNode** statements;
    size_t count;
} ProgramData;

typedef struct {
    char* name;
    char** param_names;
    char** param_types;
    size_t param_count;
    char* return_type;
    ASTNode** body;
    size_t body_count;
    int is_static;
} FunctionData;

typedef struct {
    char* name;
    char* base_class;
    ASTNode** body;
    size_t body_count;
} ClassData;

typedef struct {
    char* name;
    char* var_type;
    ASTNode* value;
} VarDeclData;

typedef struct {
    char* target;
    ASTNode* value;
} AssignmentData;

typedef struct {
    ASTNode* condition;
    ASTNode** then_branch;
    size_t then_count;
    ASTNode** else_branch;
    size_t else_count;
} IfData;

typedef struct {
    ASTNode* condition;
    ASTNode** body;
    size_t body_count;
} WhileData;

typedef struct {
    char* var_name;
    ASTNode* start;
    ASTNode* end;
    ASTNode* step;
    ASTNode** body;
    size_t body_count;
} ForData;

typedef struct { ASTNode* value; } ReturnData;
typedef struct { ASTNode** args; size_t arg_count; } PrintData;
typedef struct { char* op; ASTNode* left; ASTNode* right; } BinaryOpData;
typedef struct { char* op; ASTNode* operand; } UnaryOpData;
typedef struct { double value; } NumberData;
typedef struct { char* value; } StringData;
typedef struct { int value; } BoolData;
typedef struct { char* name; } IdentifierData;
typedef struct { ASTNode** elements; size_t count; } ListData;
typedef struct { ASTNode* object; ASTNode* index; } IndexAccessData;
typedef struct { ASTNode* object; char* attribute; } AttributeAccessData;
typedef struct { ASTNode* func; ASTNode** args; size_t arg_count; } CallData;
typedef struct { char* type_name; ASTNode** expr; size_t expr_count; } SizeofData;
typedef struct { char* name; char* var_type; ASTNode* value; } StaticVarData;
typedef struct { char* name; char* var_type; ASTNode* value; } ConstData;
typedef struct { char* type_name; ASTNode** args; size_t arg_count; } NewData;
typedef struct { ASTNode* expr; } DeleteData;
typedef struct { char* module_name; } ImportData;

ASTNode* fast_parse(const char* source);
ASTNode* fast_parse_file(const char* filename);
void fast_free(ASTNode* ast);

#endif