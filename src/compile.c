// compile.c
// node tree -> instruction sequence

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "error.h"
#include "config.h"
#include "list.h"
#include "ast.h"
#include "vm.h"
#include "object.h"
#include "token.h"
#include "compile.h"

#define compile_error(fmt, ...) \
	error(COLOR_ERROR "compile-error: " COLOR_NONE fmt, ##__VA_ARGS__)

static int compile(struct VM_state* vm, Ast* ast, struct Func_state* func);
static int compile_pushk(struct VM_state* vm, struct Func_state* func, struct Token* constant);

int compile_pushk(struct VM_state* vm, struct Func_state* func, struct Token* constant) {
	list_push(vm->program, vm->program_size, I_PUSHK);
	int location = -1;
	store_constant(func, constant, &location);
	list_push(vm->program, vm->program_size, location);
	return NO_ERR;
}

int compile(struct VM_state* vm, Ast* ast, struct Func_state* func) {
	assert(vm != NULL);
	assert(func != NULL);
	struct Token* token = NULL;
	for (int i = 0; i < ast_child_count(ast); i++) {
		token = ast_get_node(ast, i);
		if (token) {
			switch (token->type) {
				case T_NUMBER: {
					compile_pushk(vm, func, token);
				}
					break;

				default:
					compile_error("%s\n", "Unknown error");
					return COMPILE_ERR;
			}
		}
	}
	return NO_ERR;
}

int compile_from_tree(struct VM_state* vm, Ast* ast) {
	assert(ast != NULL);
	assert(vm != NULL);
	if (ast_is_empty(*ast)) return NO_ERR;
	struct Func_state global_state;
	func_state_init(&global_state);
	int status = compile(vm, ast, &global_state);
	vm->global = global_state.func;
	return status;
}
