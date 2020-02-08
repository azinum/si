// object.h

#ifndef _OBJECT_H
#define _OBJECT_H

enum Object_type {
	OBJ_UNKNOWN = 0,
	OBJ_NUMBER,
};

struct Object {
	union Value {
		double number;
	} value;
	enum Object_type type;
};

#endif