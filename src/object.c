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
		case T_NUMBER: {
			obj_number number;
			char* num_string = string_new_copy(token.string, token.length);
			string_to_number(num_string, &number);
			string_free(num_string);
			object.value.number = number;
		}	
			break;

		case T_IDENTIFIER:
			object.type = T_NULL;
			break;

		case T_NULL:
			object.type = T_NULL;
			break;

		default:
			error("Invalid token type\n");
			break;
	}
	return object;
}

int func_init(struct Function* func) {
	assert(func != NULL);
	func->addr = 0;
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

		case T_NULL:
			printf(COLOR_UNDEFINED "[null]" COLOR_NONE);
			break;

		default:
			break;
	}
}