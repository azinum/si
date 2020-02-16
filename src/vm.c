// vm.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "list.h"
#include "ast.h"
#include "hash.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

static int vm_init(struct VM_state* vm);

inline int stack_push(struct VM_state* vm, struct Object object);
inline int stack_pushk(struct VM_state* vm, struct Scope* scope, int constant);
inline int stack_pushvar(struct VM_state* vm, struct Scope* scope, int var);

int vm_init(struct VM_state* vm) {
	assert(vm != NULL);
	scope_init(&vm->global_scope, NULL);
	vm->variables = NULL;
	vm->stack_top = 0;
	vm->status = NO_ERR;
	vm->program = NULL;
	vm->program_size = 0;
	return vm->status;
}

int stack_push(struct VM_state* vm, struct Object object) {
	return NO_ERR;
}

int stack_pushk(struct VM_state* vm, struct Scope* scope, int constant) {
	return NO_ERR;
}

int stack_pushvar(struct VM_state* vm, struct Scope* scope, int var) {
	return NO_ERR;
}

int vm_dispatch(struct VM_state* vm) {
	return NO_ERR;
}

int vm_exec(char* input) {
	assert(input != NULL);
	struct VM_state vm = {};
	int status = NO_ERR;
	vm_init(&vm);

	Ast ast = ast_create();
	status = parser_parse(input, &ast);
	if (status == NO_ERR) {
		status = compile_from_tree(&vm, &ast, 0);
		if (status == NO_ERR) vm_dispatch(&vm);
	}
	ast_print(ast);
	ast_free(&ast);
	return vm.status;
}