// jumptable.h

#ifndef _JUMPTABLE_H
#define _JUMPTABLE_H

#undef vmdispatch
#undef vmcase
#undef vmbreak

#define vmdispatch(instruction) goto *jumptable[instruction];
#define vmcase(c) J_##c:
#define vmbreak vmfetch(); vmdispatch(i)

// From enum VM_instructions (vm.h)
static void* jumptable[INSTRUCTION_COUNT] = {
	&&J_I_UNKNOWN,
	&&J_I_ADD,
	&&J_I_SUB,
	&&J_I_MULT,
	&&J_I_DIV,
	&&J_I_LT,
	&&J_I_GT,
	&&J_I_EQ,
	&&J_I_LEQ,
	&&J_I_GEQ,
	&&J_I_NEQ,
	&&J_I_MOD,
	&&J_I_BAND,
	&&J_I_BOR,
	&&J_I_BXOR,
	&&J_I_LEFTSHIFT,
	&&J_I_RIGHTSHIFT,
	&&J_I_AND,
	&&J_I_OR,
	&&J_I_NOT,

	&&J_I_ASSIGN,
	&&J_I_PUSHK,
	&&J_I_POP,
	&&J_I_PUSH_VAR,
	&&J_I_RETURN,

	&&J_I_EXIT
};

#endif