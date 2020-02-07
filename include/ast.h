// ast.h

#ifndef _AST_H
#define _AST_H

#include "token.h"

struct Node {
	struct Node** children;
	int child_count;
	struct Token value;
};

typedef struct Node* Ast;

Ast create_ast();

int add_ast_node(Ast* ast, struct Token value);

int add_ast_node_at(Ast* ast, int index, struct Token value);

void print_ast(const Ast ast);

void free_ast(Ast* ast);

#endif