// object.h

#ifndef _OBJECT_H
#define _OBJECT_H

#include "hash.h"
#include "config.h"
#include "token.h"

#define MAX_ARGC 12

struct VM_state;

struct Scope {
	unsigned int constants_count;
	struct Object* constants;
	Htable var_locations;
	struct Scope* parent;
};

struct Function {
	int addr;
	int stack_offset;
	struct Scope scope;
};

// Compile-time function state
struct Func_state {
	struct Function func;
	int argc;
	int arg_types[MAX_ARGC];
	int return_type;
};

struct Object {
	union value {
		obj_number number;
		struct Function func;
	} value;
	int type;
};

struct Object token_to_object(struct Token token);

int scope_init(struct Scope* scope, struct Scope* parent);

int func_state_init(struct Func_state* state);

void object_print(const struct Object* object);

#endif