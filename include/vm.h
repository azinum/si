// vm.h

#ifndef _VM_H
#define _VM_H

enum VM_instructions {
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
	I_PUSH,
	I_POP,
	I_PUSH_VAR,

	I_EXIT
};

struct Ins_seq {
	int* sequence;
	int count;
};

int vm_exec(char* input);

#endif