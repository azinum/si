// object.h

#ifndef _OBJECT_H
#define _OBJECT_H

#include "hash.h"

enum Object_type {
	OBJ_UNKNOWN = 0,
	OBJ_NUMBER,
	OBJ_FUNC,
};

struct Scope {
	int constants_count;
	struct Object* constants;
	Htable var_locations;
	struct Scope* parent;
};

struct Func_state {
	int argc;
	int stack_offset;
	enum Object_type return_type;
	struct Scope scope;
};

struct Object {
	union value {
		double number;
		struct Func_state function;
	} value;
	enum Object_type type;
};

#endif