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
};

// TODO: Add more useful information when printing errors.
// i.e. add info about where the error occured e.t.c.
// Just like we did in the lexer and parser.
#define compile_error(fmt, ...) \
	error(COLOR_ERROR "compile-error: " COLOR_NONE fmt, ##__VA_ARGS__)

static int add_instruction(struct VM_state* vm, Instruction instruction, unsigned int* ins_count);
static int func_state_init(struct Func_state* state);
static int compile(struct VM_state* vm, Ast* ast, struct Func_state* state, unsigned int* ins_count);
static int compile_pushk(struct VM_state* vm, struct Func_state* state, struct Token constant, unsigned int* ins_count);
static int compile_pushvar(struct VM_state* vm, struct Func_state* state, struct Token variable, unsigned int* ins_count);
static int compile_declvar(struct VM_state* vm, struct Func_state* state, struct Token variable);
static int get_variable_location(struct VM_state* vm, struct Func_state* state, struct Token variable, Instruction* location);
static int store_constant(struct Func_state* state, struct Token constant, Instruction* location);
static int store_variable(struct VM_state* vm, struct Func_state* state, struct Token variable, Instruction* location);
static int token_to_op(struct Token token);
static int equal_type(const struct Token* left, const struct Token* right);

static int add_instruction(struct VM_state* vm, Instruction instruction, unsigned int* ins_count) {
	list_push(vm->program, vm->program_size, instruction);
	if (ins_count)
		(*ins_count)++;
	return NO_ERR;
}

int func_state_init(struct Func_state* state) {
	assert(state != NULL);
	state->argc = 0;
	return NO_ERR;
}

int compile_pushk(struct VM_state* vm, struct Func_state* state, struct Token constant, unsigned int* ins_count) {
	Instruction location = -1;
	store_constant(state, constant, &location);
	add_instruction(vm, I_PUSHK, ins_count);
	add_instruction(vm, location, ins_count);
	return NO_ERR;
}

int compile_pushvar(struct VM_state* vm, struct Func_state* state, struct Token variable, unsigned int* ins_count) {
	struct Scope* scope = &state->func.scope;
	char* identifier = string_new_copy(variable.string, variable.length);
	const int* found = ht_lookup(&scope->var_locations, identifier);
	Instruction location = -1;
	string_free(identifier);
	if (!found) {
		compile_error("Undeclared identifier '%.*s'\n", variable.length, variable.string);
		return COMPILE_ERR;
	}
	// Okay, no errors. Let's continue compiling!
	location = *found;
	add_instruction(vm, I_PUSH_VAR, ins_count);
	add_instruction(vm, location, ins_count);
	assert(location >= 0);
	return NO_ERR;
}

int compile_declvar(struct VM_state* vm, struct Func_state* state, struct Token variable) {
	Instruction location;
	int err = store_variable(vm, state, variable, &location);
	if (err != NO_ERR) {
		compile_error("Variable '%.*s' is already declared\n", variable.length, variable.string);
		return err;
	}
	return NO_ERR;
}

int get_variable_location(struct VM_state* vm, struct Func_state* state, struct Token variable, Instruction* location) {
	struct Scope* scope = &state->func.scope;
	char* identifier = string_new_copy(variable.string, variable.length);
	const int* found = ht_lookup(&scope->var_locations, identifier);
	string_free(identifier);
	if (!found) {
		compile_error("No such variable '%.*s'\n", variable.length, variable.string);
		return COMPILE_ERR;
	}
	*location = *found;
	return NO_ERR;
}

int store_constant(struct Func_state* state, struct Token constant, Instruction* location) {
	assert(location != NULL);
	struct Scope* scope = &state->func.scope;
	*location = scope->constants_count;
	struct Object object = token_to_object(constant);
	list_push(scope->constants, scope->constants_count, object);
	return NO_ERR;
}

int store_variable(struct VM_state* vm, struct Func_state* state, struct Token variable, Instruction* location) {
	assert(location != NULL);
	struct Scope* scope = &state->func.scope;
	char* identifier = string_new_copy(variable.string, variable.length);
	if (ht_element_exists(&scope->var_locations, identifier)) {
		string_free(identifier);
		return ERR;
	}
	else {
		struct Object object = token_to_object(variable);
		*location = vm->variable_count;
		ht_insert_element(&scope->var_locations, identifier, *location);
		list_push(vm->variables, vm->variable_count, object);
		assert(ht_element_exists(&scope->var_locations, identifier) != 0);
		string_free(identifier);
	}
	return NO_ERR;
}

#define OP_CASE(OP) case T_##OP: return I_##OP

int token_to_op(struct Token token) {
	switch (token.type) {
		OP_CASE(ADD);
		OP_CASE(SUB);
		OP_CASE(MULT);
		OP_CASE(DIV);
		OP_CASE(LT);
		OP_CASE(GT);
		OP_CASE(EQ);
		OP_CASE(LEQ);
		OP_CASE(GEQ);
		OP_CASE(NEQ);
		OP_CASE(MOD);
		OP_CASE(BAND);
		OP_CASE(BOR);
		OP_CASE(BXOR);
		OP_CASE(LEFTSHIFT);
		OP_CASE(RIGHTSHIFT);
		OP_CASE(AND);
		OP_CASE(OR);
		OP_CASE(NOT);
		default:
			break;
	}
	return I_UNKNOWN;
}

int equal_type(const struct Token* left, const struct Token* right) {
	assert(left != NULL && right != NULL);
	return left->type == right->type;
}

int compile(struct VM_state* vm, Ast* ast, struct Func_state* state, unsigned int* ins_count) {
	assert(ins_count != NULL);
	assert(vm != NULL);
	assert(state != NULL);
	struct Token* token = NULL;
	for (int i = 0; i < ast_child_count(ast); i++) {
		token = ast_get_node(ast, i);
		if (token) {
			switch (token->type) {
				// {number}
				case T_NUMBER:
					compile_pushk(vm, state, *token, ins_count);
					break;

				case T_IDENTIFIER: {
					int result = compile_pushvar(vm, state, *token, ins_count);
					if (result != NO_ERR)
						return result;
				}
					break;

				// {decl, identifier}
				case T_DECL: {
					struct Token* identifier = ast_get_node(ast, ++i);
					assert(identifier != NULL);
					int result = compile_declvar(vm, state, *identifier);
					if (result != NO_ERR)
						return result;
				}
					break;

				case T_ASSIGN: {
					struct Token* identifier = ast_get_node(ast, ++i);
					assert(identifier != NULL);
					Instruction location;
					int result = get_variable_location(vm, state, *identifier, &location);
					if (result != NO_ERR)
						return result;
					add_instruction(vm, I_ASSIGN, ins_count);
					add_instruction(vm, location, ins_count);
				}
					break;

				case T_RETURN:
					add_instruction(vm, I_RETURN, ins_count);
					break;

				default: {
					(void)equal_type;
					int op = token_to_op(*token);
					if (op != I_UNKNOWN) {
						add_instruction(vm, op, ins_count);
						break;
					}
					compile_error("%s\n", "Invalid instruction");
					return COMPILE_ERR;
				}
			}
		}
	}
	add_instruction(vm, I_RETURN, ins_count);
	return NO_ERR;
}

int compile_from_tree(struct VM_state* vm, Ast* ast) {
	assert(ast != NULL);
	assert(vm != NULL);
	if (ast_is_empty(*ast)) return NO_ERR;
	struct Func_state global_state;
	func_state_init(&global_state);
	global_state.func = vm->global;
	unsigned int ins_count = 0;
	int status = compile(vm, ast, &global_state, &ins_count);
	printf("ins_count: %i\n", ins_count);
	vm->global = global_state.func;
	return status;
}
