// vm.h

#ifndef _VM_H
#define _VM_H

#include "hash.h"
#include "object.h"

#define STACK_SIZE 128

#define INS(T, I) T##_##I,

#define INSTRUCTIONS(T) \
	INS(T, UNKNOWN) \
	INS(T, ADD) \
	INS(T, SUB) \
	INS(T, MULT) \
	INS(T, DIV) \
	INS(T, LT) \
	INS(T, GT) \
	INS(T, EQ) \
	INS(T, LEQ) \
	INS(T, GEQ) \
	INS(T, NEQ) \
	INS(T, MOD) \
	INS(T, BAND) \
	INS(T, BOR) \
	INS(T, BXOR) \
	INS(T, LEFTSHIFT) \
	INS(T, RIGHTSHIFT) \
	INS(T, AND) \
	INS(T, OR) \
	INS(T, NOT) \
\
	INS(T, ASSIGN) \
	INS(T, PUSHK) \
	INS(T, POP) \
	INS(T, PUSH_VAR) \
	INS(T, RETURN) \
	INS(T, IF) \
	INS(T, WHILE) \
	INS(T, JUMP) \
\
	INS(T, EXIT) \

enum VM_instructions {
	INSTRUCTIONS(I)

	I_BREAKJUMP,	// Dummy instruction
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