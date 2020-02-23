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
static int compile_pushk(struct VM_state* vm, struct Func_state* func, struct Token constant);
static int token_to_op(struct Token token);
static int equal_type(const struct Token* left, const struct Token* right);

int compile_pushk(struct VM_state* vm, struct Func_state* func, struct Token constant) {
	list_push(vm->program, vm->program_size, I_PUSHK);
	int location = -1;
	store_constant(func, constant, &location);
	list_push(vm->program, vm->program_size, location);
	return NO_ERR;
}

int token_to_op(struct Token token) {
	switch (token.type) {
		case T_ADD: return I_ADD;
		case T_SUB: return I_SUB;
		case T_MULT: return I_MULT;
		case T_DIV: return I_DIV;
		case T_LT: return I_LT;
		case T_GT: return I_GT;
		case T_EQ: return I_EQ;
		case T_LEQ: return I_LEQ;
		case T_GEQ: return I_GEQ;
		case T_NEQ: return I_NEQ;
		case T_NOT: return I_NOT;
		default: break;
	}
	assert(0);	// TEMP
	return I_UNKNOWN;
}

int equal_type(const struct Token* left, const struct Token* right) {
	assert(left != NULL && right != NULL);
	return left->type == right->type;
}

int compile(struct VM_state* vm, Ast* ast, struct Func_state* func) {
	assert(vm != NULL);
	assert(func != NULL);
	struct Token* token = NULL;
	for (int i = 0; i < ast_child_count(ast); i++) {
		token = ast_get_node(ast, i);
		if (token) {
			switch (token->type) {
				case T_NUMBER:
					compile_pushk(vm, func, *token);
					break;

				case T_ADD:
				case T_SUB:
				case T_MULT:
				case T_DIV:
				case T_LT:
				case T_GT:
				case T_EQ:
				case T_LEQ:
				case T_GEQ:
				case T_NEQ: {
					struct Token* left = ast_get_node(ast, i - 2);
					struct Token* right = ast_get_node(ast, i - 1);
					if (!equal_type(left, right)) {
						compile_error("%s\n", "Expected equal types");
						return COMPILE_ERR;
					}
					int op = token_to_op(*token);
					list_push(vm->program, vm->program_size, op);
				}
					break;

				case T_NOT: {
					int op = token_to_op(*token);
					list_push(vm->program, vm->program_size, op);
				}
					break;

				default:
					compile_error("%s\n", "Invalid instruction");
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
