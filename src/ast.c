// ast.c

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "error.h"
#include "ast.h"

struct Node {
	struct Node** children;
	int child_count;
	Value value;
};

static int is_empty(const Ast ast);
static struct Node* create_node(Value value);
static int print_tree(const Ast ast, int level);

int is_empty(const Ast ast) {
	return ast == NULL;
}

struct Node* create_node(Value value) {
	struct Node* node = malloc(sizeof(struct Node));
	if (!node) {
		error("Failed to allocate new AST node\n");
		return NULL;
	}

	node->value = value;
	node->child_count = 0;
	node->children = NULL;

	return node;
}

int print_tree(const Ast ast, int level) {
	if (is_empty(ast))
		return NO_ERR;
	for (int i = 0; i < level; i++) {
		printf(" ");
	}

	printf("%.*s\n", ast->value.length, ast->value.string);

	for (unsigned long i = 0; i < ast->child_count; i++) {
		print_tree(ast->children[i], level + 1);
	}

	return NO_ERR;
}

Ast create_ast() {
	return NULL;
}

int ast_is_empty(const Ast ast) {
	return is_empty(ast);
}

int add_ast_node(Ast* ast, Value value) {
	struct Node* new_node = create_node(value);
	if (!new_node)
		return ALLOC_ERR;

	if (is_empty(*ast)) {
		*ast = create_node((struct Token) {});	// Create root node
		if (!(*ast))
			return ALLOC_ERR;
	}
	if (!(*ast)->children) {
		(*ast)->children = malloc(sizeof(struct Node));
	}
	else {
		struct Node** tmp = realloc((*ast)->children, (sizeof(struct Node*)) * (*ast)->child_count + 1);
		if (!tmp) return REALLOC_ERR;
		(*ast)->children = tmp;
	}
	(*ast)->children[(*ast)->child_count++] = new_node;
	return NO_ERR;
}

int add_ast_node_at(Ast* ast, int index, Value value) {
	assert(!is_empty(*ast));
	assert(index < (*ast)->child_count);
	return add_ast_node(&(*ast)->children[index], value);
}

void print_ast(const Ast ast) {
	if (is_empty(ast)) return;
	print_tree(ast, 0);
}

void free_ast(Ast* ast) {
	assert(ast != NULL);
	if (is_empty(*ast)) return;

	for (unsigned long i = 0; i < (*ast)->child_count; i++)
		free_ast(&(*ast)->children[i]);

	free((*ast)->children);
	(*ast)->children = NULL;
	(*ast)->child_count = 0;
	free(*ast);
	*ast = NULL;
	assert(is_empty(*ast));
}