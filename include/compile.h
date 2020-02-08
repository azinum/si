// compile.h

#ifndef _COMPILE_H
#define _COMPILE_H

typedef struct Node* Ast;
struct Ins_seq;	// vm.h

int compile_from_tree(struct Ins_seq* seq, Ast* ast, int level);

#endif