// vm.h

#ifndef _VM_H
#define _VM_H

#include "hash.h"
#include "object.h"

#define STACK_SIZE 128

enum VM_instructions {
	I_UNKNOWN,
	I_ADD = 1,
	I_SUB,
	I_MULT,
	I_DIV,
	I_LT,
	I_GT,
	I_EQ,
	I_LEQ,
	I_GEQ,
	I_NEQ,

	I_ASSIGN,
	I_PUSHK,
	I_POP,
	I_PUSH_VAR,

	I_EXIT
};

struct VM_state {
	struct Function global;
	struct Object* variables;
	int variables_count;
	struct Object stack[STACK_SIZE];
	int stack_top;
	int status;
	int* program;
	int program_size;
};

int vm_exec(char* input);

#endif