// compile.h

#ifndef _COMPILE_H
#define _COMPILE_H

typedef struct Node* Ast;
struct VM_state;
struct Scope;

int scope_init(struct Scope* scope, struct Scope* parent);

int compile_from_tree(struct VM_state* vm, Ast* ast, int level);

#endif