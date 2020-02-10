// vm.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "ast.h"
#include "hash.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

static int vm_init(struct VM_state* vm);

int vm_init(struct VM_state* vm) {
	assert(vm != NULL);
	vm->status = NO_ERR;
	return vm->status;
}

int vm_exec(char* input) {
	assert(input != NULL);
	struct VM_state vm = {};
	int status = NO_ERR;
	vm_init(&vm);

	Ast ast = create_ast();
	status = parser_parse(input, &ast);
	if (status == NO_ERR) {
		status = compile_from_tree(&vm.seq, &ast, 0);
	}
	print_ast(ast);
	free_ast(&ast);
	return vm.status;
}