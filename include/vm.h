// vm.h

#ifndef _VM_H
#define _VM_H

#include "hash.h"
#include "object.h"

#define STACK_SIZE 128

typedef int Instruction;

#define INSTRUCTIONS(T) \
	T##_UNKNOWN, \
	T##_ADD, \
	T##_SUB, \
	T##_MULT, \
	T##_DIV, \
	T##_LT, \
	T##_GT, \
	T##_EQ, \
	T##_LEQ, \
	T##_GEQ, \
	T##_NEQ, \
	T##_MOD, \
	T##_BAND, \
	T##_BOR, \
	T##_BXOR, \
	T##_LEFTSHIFT, \
	T##_RIGHTSHIFT, \
	T##_AND, \
	T##_OR, \
	T##_NOT, \
\
	T##_ASSIGN, \
	T##_PUSHK, \
	T##_POP, \
	T##_PUSH_VAR, \
	T##_RETURN, \
	T##_IF, \
	T##_WHILE, \
	T##_JUMP, \
\
	T##_EXIT, \

enum VM_instructions {
	INSTRUCTIONS(I)
	INSTRUCTION_COUNT
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

int vm_disasm(struct VM_state* vm, const char* output_file);

void vm_state_free(struct VM_state* vm);

#endif