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

Ast ast_get_node_at(Ast* ast, int index);

Ast ast_get_last(Ast* ast);

int ast_remove_node_at(Ast* ast, int index);

int ast_child_count(const Ast* ast);

int ast_child_count_total(const Ast* ast);

Ast* ast_get_node(Ast* ast, int index);

Value* ast_get_node_value(Ast* ast, int index);

void ast_print(const Ast ast);

void ast_free(Ast* ast);

#endif
