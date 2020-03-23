// compile.h

#ifndef _COMPILE_H
#define _COMPILE_H

#include "config.h"

typedef struct Node* Ast;
struct VM_state;

int compile_from_tree(struct VM_state* vm, Ast* ast);

unsigned int compile_get_ins_arg_count(Instruction instruction);

#endif