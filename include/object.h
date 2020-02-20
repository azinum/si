// object.h

#ifndef _OBJECT_H
#define _OBJECT_H

#include "hash.h"

#define MAX_ARGC 12

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

struct Function {
	int addr;
	int stack_offset;
	struct Scope scope;
};

// Compile-time function state
struct Func_state {
	struct Function func;
	int argc;
	enum Object_type arg_types[MAX_ARGC];
	enum Object_type return_type;
};

struct Object {
	union value {
		double number;
		struct Function func;
	} value;
	enum Object_type type;
};

#endif