// stack.h

#ifndef _STACK_H
#define _STACK_H

extern int stack_push(struct VM_state* vm, struct Object object);
extern int stack_pop(struct VM_state* vm);
extern int stack_pushk(struct VM_state* vm, struct Scope* scope, int constant);
extern int stack_pushvar(struct VM_state* vm, struct Scope* scope, int var);
extern int stack_reset(struct VM_state* vm);
extern int stack_print_top(struct VM_state* vm);
extern struct Object* stack_gettop(struct VM_state* vm);
extern struct Object* stack_get(struct VM_state* vm, int offset);

#endif
