// api.c

#include <stdio.h>

#include "api.h"

int si_store_object(struct VM_state* vm, struct Scope* scope, const char* name, struct Object object) {
  if (ht_element_exists(&scope->var_locations, name)) {
    return ERR;
  }
  Instruction location = vm->variable_count;
  ht_insert_element(&scope->var_locations, name, location);
  list_push(vm->variables, vm->variable_count, object);
  return NO_ERR;
}

int si_store_number(struct VM_state* vm, const char* name, double number) {
  struct Scope* scope = &vm->global.scope;
  struct Object object = {
    .type = T_NUMBER,
    .value.number = number
  };
  return si_store_object(vm, scope, name, object);
}

int si_store_cfunc(struct VM_state* vm, const char* name, CFunction cfunc) {
  struct Scope* scope = &vm->global.scope;
  struct Object object = {
    .type = T_CFUNCTION,
    .value.cfunc = cfunc
  };
  return si_store_object(vm, scope, name, object);
}

int si_push_number(struct VM_state* vm, obj_number number) {
  struct Object obj = (struct Object) {
    .type = T_NUMBER,
    .value.number = number
  };
  stack_push(vm, obj);
  return 0;
}

int si_get_argc(struct VM_state* vm) {
  return vm->stack_top - vm->stack_bp;
}

struct Object* si_get_arg(struct VM_state* vm, int num_arg) {
  struct Object* arg = NULL;
  arg = &vm->stack[vm->stack_bp + num_arg];
  return arg;
}
