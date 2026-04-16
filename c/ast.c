#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* create_node(ASTNodeType type, void* data) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->data = data;
    return node;
}

static void free_data(ASTNode* node) {
    if (!node || !node->data) return;
    
    switch (node->type) {
        case AST_PROGRAM: {
            ProgramData* d = node->data;
            for (size_t i = 0; i < d->count; i++) free_ast(d->statements[i]);
            free(d->statements);
            break;
        }
        case AST_FUNCTION: {
            FunctionData* d = node->data;
            free(d->name);
            for (size_t i = 0; i < d->param_count; i++) {
                free(d->param_names[i]);
                free(d->param_types[i]);
            }
            free(d->param_names);
            free(d->param_types);
            free(d->return_type);
            for (size_t i = 0; i < d->body_count; i++) free_ast(d->body[i]);
            free(d->body);
            break;
        }
        case AST_CLASS: {
            ClassData* d = node->data;
            free(d->name);
            free(d->base_class);
            for (size_t i = 0; i < d->body_count; i++) free_ast(d->body[i]);
            free(d->body);
            break;
        }
        case AST_VAR_DECL: {
            VarDeclData* d = node->data;
            free(d->name);
            free(d->var_type);
            if (d->value) free_ast(d->value);
            break;
        }
        case AST_ASSIGNMENT: {
            AssignmentData* d = node->data;
            free(d->target);
            if (d->value) free_ast(d->value);
            break;
        }
        case AST_IF: {
            IfData* d = node->data;
            if (d->condition) free_ast(d->condition);
            for (size_t i = 0; i < d->then_count; i++) free_ast(d->then_branch[i]);
            free(d->then_branch);
            if (d->else_branch) {
                for (size_t i = 0; i < d->else_count; i++) free_ast(d->else_branch[i]);
                free(d->else_branch);
            }
            break;
        }
        case AST_WHILE: {
            WhileData* d = node->data;
            if (d->condition) free_ast(d->condition);
            for (size_t i = 0; i < d->body_count; i++) free_ast(d->body[i]);
            free(d->body);
            break;
        }
        case AST_FOR: {
            ForData* d = node->data;
            free(d->var_name);
            if (d->start) free_ast(d->start);
            if (d->end) free_ast(d->end);
            if (d->step) free_ast(d->step);
            for (size_t i = 0; i < d->body_count; i++) free_ast(d->body[i]);
            free(d->body);
            break;
        }
        case AST_RETURN: {
            ReturnData* d = node->data;
            if (d->value) free_ast(d->value);
            break;
        }
        case AST_PRINT: {
            PrintData* d = node->data;
            for (size_t i = 0; i < d->arg_count; i++) free_ast(d->args[i]);
            free(d->args);
            break;
        }
        case AST_BINARY_OP: {
            BinaryOpData* d = node->data;
            free(d->op);
            if (d->left) free_ast(d->left);
            if (d->right) free_ast(d->right);
            break;
        }
        case AST_UNARY_OP: {
            UnaryOpData* d = node->data;
            free(d->op);
            if (d->operand) free_ast(d->operand);
            break;
        }
        case AST_NUMBER:
        case AST_BOOL:
        case AST_NONE:
        case AST_NULLPTR:
            break;
        case AST_STRING: {
            StringData* d = node->data;
            free(d->value);
            break;
        }
        case AST_IDENTIFIER: {
            IdentifierData* d = node->data;
            free(d->name);
            break;
        }
        case AST_LIST: {
            ListData* d = node->data;
            for (size_t i = 0; i < d->count; i++) free_ast(d->elements[i]);
            free(d->elements);
            break;
        }
        case AST_INDEX_ACCESS: {
            IndexAccessData* d = node->data;
            if (d->object) free_ast(d->object);
            if (d->index) free_ast(d->index);
            break;
        }
        case AST_ATTRIBUTE_ACCESS: {
            AttributeAccessData* d = node->data;
            if (d->object) free_ast(d->object);
            free(d->attribute);
            break;
        }
        case AST_CALL: {
            CallData* d = node->data;
            if (d->func) free_ast(d->func);
            for (size_t i = 0; i < d->arg_count; i++) free_ast(d->args[i]);
            free(d->args);
            break;
        }
        case AST_SIZEOF: {
            SizeofData* d = node->data;
            free(d->type_name);
            for (size_t i = 0; i < d->expr_count; i++) free_ast(d->expr[i]);
            free(d->expr);
            break;
        }
        case AST_STATIC_VAR: {
            StaticVarData* d = node->data;
            free(d->name);
            free(d->var_type);
            if (d->value) free_ast(d->value);
            break;
        }
        case AST_CONST: {
            ConstData* d = node->data;
            free(d->name);
            free(d->var_type);
            if (d->value) free_ast(d->value);
            break;
        }
        case AST_NEW: {
            NewData* d = node->data;
            free(d->type_name);
            for (size_t i = 0; i < d->arg_count; i++) free_ast(d->args[i]);
            free(d->args);
            break;
        }
        case AST_DELETE: {
            DeleteData* d = node->data;
            if (d->expr) free_ast(d->expr);
            break;
        }
        case AST_BREAK:
        case AST_CONTINUE:
        case AST_ADDRESS_OF:
        case AST_DEREFERENCE:
            break;
    }
    free(node->data);
}

void free_ast(ASTNode* node) {
    if (!node) return;
    free_data(node);
    free(node);
}