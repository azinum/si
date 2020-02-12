// ast.h

#ifndef _AST_H
#define _AST_H

#include "token.h"

typedef struct Token Value;

typedef struct Node* Ast;

Ast ast_create();

int ast_is_empty(const Ast ast);

int ast_add_node(Ast* ast, Value value);

int ast_add_node_at(Ast* ast, int index, Value value);

int ast_child_count(Ast* ast);

Value* ast_get_node(Ast* ast, int index);

void ast_print(const Ast ast);

void ast_free(Ast* ast);

#endif