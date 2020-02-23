// object.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "list.h"
#include "vm.h"
#include "token.h"
#include "str.h"
#include "object.h"

static int func_init(struct Function* func);
static struct Object token_to_object(struct Token token);

int func_init(struct Function* func) {
	assert(func != NULL);
	func->addr = -1;
	func->stack_offset = 0;
	scope_init(&func->scope, NULL);
	return NO_ERR;
}

struct Object token_to_object(struct Token token) {
	struct Object object = { .type = token.type };
	switch (token.type) {
		case T_NUMBER: {
			obj_number number;
			char* num_string = string_new_copy(token.string, token.length);
			string_to_number(num_string, &number);
			string_free(num_string);
			object.value.number = number;
		}	
			break;

		default:
			error("Invalid token type\n");
			break;
	}
	return object;
}

int scope_init(struct Scope* scope, struct Scope* parent) {
	assert(scope != NULL);
	scope->constants_count = 0;
	scope->constants = NULL;
	scope->var_locations = ht_create_empty();
	scope->parent = parent;
	return NO_ERR;
}

int func_state_init(struct Func_state* state) {
	assert(state != NULL);
	state->argc = 0;
	state->return_type = T_UNKNOWN;
	func_init(&state->func);
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

void object_print(struct Object object) {
	switch (object.type) {
		case T_NUMBER:
			printf("%g\n", object.value.number);
			break;
		default:
			break;
	}
}