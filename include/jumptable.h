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
	INSTRUCTIONS(&&J_I)
};

#endif
