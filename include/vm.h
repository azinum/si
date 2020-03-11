// vm.h

#ifndef _VM_H
#define _VM_H

#include "hash.h"
#include "object.h"

#define STACK_SIZE 128

typedef int Instruction;

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
	I_MOD,
	I_BAND,
	I_BOR,
	I_BXOR,
	I_LEFTSHIFT,
	I_RIGHTSHIFT,
	I_AND,
	I_OR,
	I_NOT,

	I_ASSIGN,
	I_PUSHK,
	I_POP,
	I_PUSH_VAR,
	I_RETURN,

	I_EXIT
};

struct VM_state {
	struct Function global;
	struct Object* variables;
	int variable_count;
	struct Object stack[STACK_SIZE];
	int stack_top;
	int status;
	Instruction* program;
	int program_size;
	int prev_ip;	// Instruction pointer from the previous vm dispatch
	int heap_allocated;	// Is the vm state heap allocated?
};

int vm_init(struct VM_state* vm);

struct VM_state* vm_state_new();

int vm_exec(struct VM_state* vm, char* input);

void vm_state_free(struct VM_state* vm);

#endif