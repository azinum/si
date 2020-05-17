// stack.c - stack manipulation functions

#include <stdlib.h>
#include <assert.h>

#include "vm.h"
#include "error.h"
#include "object.h"
#include "stack.h"

inline int stack_push(struct VM_state* vm, struct Object object) {
  if (vm->stack_top >= STACK_SIZE) {
    vmerror("Stack overflow\n");
    return vm->status = STACK_ERR;
  }
  vm->stack[vm->stack_top++] = object;
  return NO_ERR;
}

inline int stack_pop(struct VM_state* vm) {
  if (vm->stack_top <= 0) {
    vmerror("Can't pop stack\n");
    return vm->status = STACK_ERR;
  }
  vm->stack_top--;
  return NO_ERR;
}

inline int stack_pushk(struct VM_state* vm, struct Scope* scope, int constant) {
  assert(scope->constants_count > constant);
  struct Object object = scope->constants[constant];
  return stack_push(vm, object);
}

inline int stack_pushvar(struct VM_state* vm, struct Scope* scope, int var) {
  assert(vm->variable_count > var);
  struct Object object = vm->variables[var];
  return stack_push(vm, object);
}

inline int stack_reset(struct VM_state* vm) {
  vm->stack_top = 0;
  return NO_ERR;
}

inline int stack_print_top(struct VM_state* vm) {
  struct Object* top = stack_gettop(vm);
  if (top)
    object_printline(top);
  return NO_ERR;
}

inline struct Object* stack_gettop(struct VM_state* vm) {
  return stack_get(vm, 0);
}

inline struct Object* stack_get(struct VM_state* vm, int offset) {
  int index = vm->stack_top - (offset + 1); // offset 0 is top of stack
  if (index >= 0)
    return &vm->stack[index];
  return NULL;
}
