// vm.c

#include <assert.h>
#include <stdlib.h>

#include "error.h"
#include "ast.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

struct VM_state {

};

int vm_exec(char* input) {
	assert(input != NULL);
	struct VM_state vm = {};
	int status = NO_ERR;

	Ast ast = create_ast();
	status = parser_parse(input, &ast);
	if (status == NO_ERR) {
		status = compile_from_tree(&vm, &ast, 0);
	}
	free_ast(&ast);
	return status;
}