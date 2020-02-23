// vm.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "config.h"
#include "list.h"
#include "ast.h"
#include "hash.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

inline int stack_push(struct VM_state* vm, struct Object object);
inline int stack_pushk(struct VM_state* vm, struct Scope* scope, int constant);
inline int stack_pushvar(struct VM_state* vm, struct Scope* scope, int var);

static int vm_init(struct VM_state* vm);
static int vm_dispatch(struct VM_state* vm, struct Function* func);

int stack_push(struct VM_state* vm, struct Object object) {
	return NO_ERR;
}

int stack_pushk(struct VM_state* vm, struct Scope* scope, int constant) {
	return NO_ERR;
}

int stack_pushvar(struct VM_state* vm, struct Scope* scope, int var) {
	return NO_ERR;
}

int vm_init(struct VM_state* vm) {
	assert(vm != NULL);
	scope_init(&vm->global.scope, NULL);
	vm->variables = NULL;
	vm->stack_top = 0;
	vm->status = NO_ERR;
	vm->program = NULL;
	vm->program_size = 0;
	return vm->status;
}

int vm_dispatch(struct VM_state* vm, struct Function* func) {
	return NO_ERR;
}

int vm_exec(char* input) {
	assert(input != NULL);
	struct VM_state vm = {};
	int status = NO_ERR;
	vm_init(&vm);

	Ast ast = ast_create();
	status = parser_parse(input, &ast);
	printf(COLOR_MESSAGE "(parser)" COLOR_NONE " status code: %i\n", status);
	if (status == NO_ERR) {
		if (compile_from_tree(&vm, &ast) == NO_ERR)
			vm_dispatch(&vm, &vm.global);
		ast_print(ast);
	}
	ast_free(&ast);
	return vm.status;
}