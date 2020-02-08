// compile.h

#ifndef _COMPILE_H
#define _COMPILE_H

struct VM_state;
typedef struct Node* Ast;

int compile_from_tree(struct VM_state* vm, Ast* ast, int level);

#endif