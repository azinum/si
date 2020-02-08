// compile.c
// node tree -> instruction sequence

#include <assert.h>
#include <stdlib.h>

#include "error.h"
#include "ast.h"
#include "vm.h"
#include "compile.h"

int compile_from_tree(struct VM_state* vm, Ast* ast, int level) {
	assert(ast != NULL);
	if (ast_is_empty(*ast)) return NO_ERR;
	return NO_ERR;
}