// compile.h

#ifndef _COMPILE_H
#define _COMPILE_H

typedef struct Node* Ast;
struct VM_state;

int compile_from_tree(struct VM_state* vm, Ast* ast, int level);

#endif