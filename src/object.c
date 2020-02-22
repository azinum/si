// object.c

#include <assert.h>
#include <stdlib.h>

#include "error.h"
#include "object.h"

static int func_init(struct Function* func);

int func_init(struct Function* func) {
	assert(func != NULL);
	func->addr = -1;
	func->stack_offset = 0;
	scope_init(&func->scope, NULL);
	return NO_ERR;
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
	state->return_type = OBJ_UNKNOWN;
	func_init(&state->func);
	return NO_ERR;
}