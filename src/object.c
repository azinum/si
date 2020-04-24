// object.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "mem.h"
#include "list.h"
#include "vm.h"
#include "token.h"
#include "str.h"
#include "object.h"

struct Object token_to_object(struct Token token) {
	struct Object object = { .type = token.type };
	switch (token.type) {
		case T_IDENTIFIER:
			object.type = T_NIL;
			break;

		case T_FUNCTION:
			object.type = T_FUNCTION;
			break;

		case T_NUMBER:
			object.value.number = token.value.number;
			break;

		case T_NIL:
			object.type = T_NIL;
			break;

		default:
			error("Invalid token type\n");
			break;
	}
	return object;
}

int func_init(struct Function* func) {
	assert(func != NULL);
  func_init_with_parent_scope(func, NULL);
	return NO_ERR;
}

int func_init_with_parent_scope(struct Function* func, struct Scope* parent) {
  assert(func != NULL);
  func->addr = 0;
  func->bp = 0;
  return scope_init(&func->scope, parent);
}

int scope_init(struct Scope* scope, struct Scope* parent) {
	assert(scope != NULL);
	scope->constants_count = 0;
	scope->constants = NULL;
	scope->var_locations = ht_create_empty();
	scope->parent = parent;
	return NO_ERR;
}

int scope_free(struct Scope* scope) {
	assert(scope != NULL);
	mfree(scope->constants, scope->constants_count * sizeof(struct Object));
	ht_free(&scope->var_locations);
	return NO_ERR;
}

void object_printline(const struct Object* object) {
	assert(object != NULL);
	object_print(object);
	printf("\n");
}

void object_print(const struct Object* object) {
	assert(object != NULL);
	switch (object->type) {
		case T_NUMBER:
			printf(COLOR_NUMBER "%g" COLOR_NONE, object->value.number);
			break;

		case T_FUNCTION:
			printf(COLOR_TYPE "[Function]" COLOR_NONE " (addr: %i)", object->value.func.addr);
			break;

		case T_NIL:
			printf(COLOR_NIL "[Nil]" COLOR_NONE);
			break;

		default:
			printf(COLOR_TYPE "[Undefined]" COLOR_NONE);
			break;
	}
}