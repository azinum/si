// compile.c
// node tree -> instruction sequence

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "error.h"
#include "config.h"
#include "str.h"
#include "hash.h"
#include "list.h"
#include "ast.h"
#include "vm.h"
#include "object.h"
#include "token.h"
#include "compile.h"

// Compile-time function state
struct Func_state {
	struct Function func;
	int argc;
	int arg_types[MAX_ARGC];
	int return_type;
};

// TODO: Add more useful information when printing errors.
// i.e. add info about where the error occured e.t.c.
// Just like we did in the lexer and parser.
#define compile_error(fmt, ...) \
	error(COLOR_ERROR "compile-error: " COLOR_NONE fmt, ##__VA_ARGS__)

static int func_state_init(struct Func_state* state);
static int compile(struct VM_state* vm, Ast* ast, struct Func_state* state);
static int compile_pushk(struct VM_state* vm, struct Func_state* state, struct Token constant);
static int compile_pushvar(struct VM_state* vm, struct Func_state* state, struct Token variable);
static int compile_declvar(struct VM_state* vm, struct Func_state* state, struct Token variable);
static int get_variable_location(struct VM_state* vm, struct Func_state* state, struct Token variable, int* location);
static int store_constant(struct Func_state* state, struct Token constant, int* location);
static int store_variable(struct VM_state* vm, struct Func_state* state, struct Token variable, int* location);
static int token_to_op(struct Token token);
static int equal_type(const struct Token* left, const struct Token* right);

int func_state_init(struct Func_state* state) {
	assert(state != NULL);
	state->argc = 0;
	state->return_type = T_UNKNOWN;
	return NO_ERR;
}

int compile_pushk(struct VM_state* vm, struct Func_state* state, struct Token constant) {
	list_push(vm->program, vm->program_size, I_PUSHK);
	int location = -1;
	store_constant(state, constant, &location);
	list_push(vm->program, vm->program_size, location);
	return NO_ERR;
}

int compile_pushvar(struct VM_state* vm, struct Func_state* state, struct Token variable) {
	struct Scope* scope = &state->func.scope;
	char* identifier = string_new_copy(variable.string, variable.length);
	const int* found = ht_lookup(&scope->var_locations, identifier);
	int location = -1;
	string_free(identifier);
	if (!found) {
		compile_error("Undeclared identifier '%.*s'\n", variable.length, variable.string);
		return COMPILE_ERR;
	}
	// Okay, no errors. Let's continue compiling!
	location = *found;
	list_push(vm->program, vm->program_size, I_PUSH_VAR);
	list_push(vm->program, vm->program_size, location);
	return NO_ERR;
}

int compile_declvar(struct VM_state* vm, struct Func_state* state, struct Token variable) {
	int location = -1;
	int err = store_variable(vm, state, variable, &location);
	if (err != NO_ERR) {
		compile_error("Variable '%.*s' already exists\n", variable.length, variable.string);
		return err;
	}
	return NO_ERR;
}

int get_variable_location(struct VM_state* vm, struct Func_state* state, struct Token variable, int* location) {
	struct Scope* scope = &state->func.scope;
	char* identifier = string_new_copy(variable.string, variable.length);
	const int* found = ht_lookup(&scope->var_locations, identifier);
	string_free(identifier);
	if (!found) {
		compile_error("No such variable '%.*s'", variable.length, variable.string);
		return COMPILE_ERR;
	}
	*location = *found;
	return NO_ERR;
}

int store_constant(struct Func_state* state, struct Token constant, int* location) {
	assert(location != NULL);
	struct Scope* scope = &state->func.scope;
	*location = scope->constants_count;
	struct Object object = token_to_object(constant);
	list_push(scope->constants, scope->constants_count, object);
	return NO_ERR;
}

int store_variable(struct VM_state* vm, struct Func_state* state, struct Token variable, int* location) {
	assert(location != NULL);
	struct Scope* scope = &state->func.scope;
	char* identifier = string_new_copy(variable.string, variable.length);
	if (ht_element_exists(&scope->var_locations, identifier)) {
		string_free(identifier);
		return ERR;
	}
	else {
		struct Object object = token_to_object(variable);
		*location = vm->variables_count;
		ht_insert_element(&scope->var_locations, identifier, *location);
		list_push(vm->variables, vm->variables_count, object);
		assert(ht_element_exists(&scope->var_locations, identifier) != 0);
		string_free(identifier);
	}
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
	assert(0);
	return I_UNKNOWN;
}

// TODO: Compare between constants and variables (when implemented)
int equal_type(const struct Token* left, const struct Token* right) {
	assert(left != NULL && right != NULL);
	return left->type == right->type;
}

int compile(struct VM_state* vm, Ast* ast, struct Func_state* state) {
	assert(vm != NULL);
	assert(state != NULL);
	struct Token* token = NULL;
	for (int i = 0; i < ast_child_count(ast); i++) {
		token = ast_get_node(ast, i);
		if (token) {
			switch (token->type) {
				// {number}
				case T_NUMBER:
					compile_pushk(vm, state, *token);
					break;

				case T_IDENTIFIER: {
					int result = compile_pushvar(vm, state, *token);
					if (result != NO_ERR)
						return result;
				}
					break;

				// {decl, identifier}
				case T_DECL: {
					int result = compile_declvar(vm, state, *token);
					if (result != NO_ERR)
						return result;
				}
					break;

				case T_ASSIGN: {
					int location;
					int result = get_variable_location(vm, state, *token, &location);
					if (result != NO_ERR)
						return result;
					list_push(vm->program, vm->program_size, I_ASSIGN);
					list_push(vm->program, vm->program_size, location);
				}
					break;

				// All of these require two operands {opr_a, opr_b, op}
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
					(void)equal_type;	// Silence warning
					int op = token_to_op(*token);
					list_push(vm->program, vm->program_size, op);
				}
					break;

				// These only require one operand {opr, uop}
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
	global_state.func = vm->global;
	int status = compile(vm, ast, &global_state);
	vm->global = global_state.func;
	return status;
}
