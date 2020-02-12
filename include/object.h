// object.h

#ifndef _OBJECT_H
#define _OBJECT_H

#include "hash.h"

enum Object_type {
	OBJ_UNKNOWN = 0,
	OBJ_NUMBER,
};

struct Scope {
	int constants_count;
	struct Object* constants;
	Htable var_locations;
	struct Object* variables;
	struct Scope* parent;
};

struct Object {
	union value {
		double number;
	} value;
	enum Object_type type;
};

struct Func_state {
	int argc;
	int stack_offset;
	enum Object_type return_type;
	struct Scope scope;
};

#endif