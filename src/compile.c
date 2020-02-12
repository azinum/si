// compile.c
// node tree -> instruction sequence

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "ast.h"
#include "vm.h"
#include "object.h"
#include "token.h"
#include "compile.h"

int compile_from_tree(struct VM_state* vm, Ast* ast, int level) {
	assert(ast != NULL);
	assert(vm != NULL);
	if (ast_is_empty(*ast)) return NO_ERR;

	for (int i = 0; i < ast_child_count(ast); i++) {
		Value* value = ast_get_node(ast, i);
		if (value) {
			print_token(*value);
		}
	}

	return NO_ERR;
}
