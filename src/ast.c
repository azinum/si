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
	for (int i = 0; i < level - 1; i++) {
		printf("  ");
	}
	if (ast->value.length > 0)
		printf("%.*s\n", ast->value.length, ast->value.string);
	else if (level > 0)
		printf("t:%i\n", ast->value.type);

	for (unsigned long i = 0; i < ast->child_count; i++) {
		print_tree(ast->children[i], level + 1);
	}

	return NO_ERR;
}

Ast ast_create() {
	return NULL;
}

int ast_is_empty(const Ast ast) {
	return is_empty(ast);
}

int ast_add_node(Ast* ast, Value value) {
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

int ast_add_node_at(Ast* ast, int index, Value value) {
	assert(!is_empty(*ast));
	assert(index < (*ast)->child_count);
	return ast_add_node(&(*ast)->children[index], value);
}

Ast ast_get_node_at(Ast* ast, int index) {
	assert(!is_empty(*ast));
	if (index < 0 || index > (*ast)->child_count)
		return NULL;
	return (*ast)->children[index];
}

Ast ast_get_last(Ast* ast) {
	assert(!is_empty(*ast));
	int child_count = ast_child_count(ast);
	if (child_count == 0)
		return NULL;
	return (*ast)->children[child_count - 1];
}

int ast_remove_node_at(Ast* ast, int index) {
	assert(!is_empty(*ast));
	int child_count = ast_child_count(ast);
	assert(index < child_count);
	Ast node_to_remove = ast_get_node_at(ast, index);
	ast_free(&node_to_remove);	// Remove the subtree(s) from this node - if there are any
	(*ast)->children[index] = NULL;
	for (int i = index; i < child_count - 1; i++) {
		(*ast)->children[i] = (*ast)->children[i + 1];
	}
	(*ast)->child_count--;
	return NO_ERR;
}

int ast_child_count(const Ast* ast) {
	assert(ast != NULL);
	if (!(*ast))
		return 0;
	return (*ast)->child_count;
}

Ast* ast_get_node(Ast* ast, int index) {
	assert(ast != NULL);
	if (index >= 0 && index < ast_child_count(ast)) {
		return &(*ast)->children[index];
	}
	return NULL;
}

Value* ast_get_node_value(Ast* ast, int index) {
	assert(ast != NULL);
	if (index >= 0 && index < ast_child_count(ast)) {
		struct Node* node = (*ast)->children[index];
		if (!node)
			return NULL;
		return &node->value;
	}
	return NULL;
}

void ast_print(const Ast ast) {
	if (is_empty(ast)) return;
	print_tree(ast, 0);
	printf("\n");
}

void ast_free(Ast* ast) {
	assert(ast != NULL);
	if (is_empty(*ast)) return;

	for (unsigned long i = 0; i < (*ast)->child_count; i++)
		ast_free(&(*ast)->children[i]);

	free((*ast)->children);
	(*ast)->children = NULL;
	(*ast)->child_count = 0;
	free(*ast);
	*ast = NULL;
	assert(is_empty(*ast));
}