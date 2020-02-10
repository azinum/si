// ast.h

#ifndef _AST_H
#define _AST_H

#include "token.h"

typedef struct Token Value;

typedef struct Node* Ast;

Ast create_ast();

int ast_is_empty(const Ast ast);

int add_ast_node(Ast* ast, Value value);

int add_ast_node_at(Ast* ast, int index, Value value);

void print_ast(const Ast ast);

void free_ast(Ast* ast);

#endif