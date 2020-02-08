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

	Ast ast = create_ast();
	parser_parse(input, &ast);
	print_ast(ast);
	free_ast(&ast);

	return NO_ERR;
}