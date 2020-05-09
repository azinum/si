// libbase.c

#include <stdlib.h>
#include <stdio.h>

#include "si.h"
#include "vm.h"
#include "object.h"
#include "hash.h"
#include "lib.h"

int print_state(struct VM_state* vm, struct Scope* scope) {
  if (!scope)
    return 0;
  printf("{\n");
  for (int i = 0; i < ht_get_size(&scope->var_locations); i++) {
    const Hkey* key = ht_lookup_key(&scope->var_locations, i);
    const Hvalue* value = ht_lookup_byindex(&scope->var_locations, i);
    if (key != NULL && value != NULL) {
      printf("  %s: ", *key);
      struct Object* object = &vm->variables[*value];
      object_print(object);
      printf(",\n");
    }
  }
  printf("}\n");
  return 0;
}

int print_recursive(struct VM_state* vm, struct Scope* scope, int level) {
  if (!scope)
    return 0;
  printf("{\n");
  for (int i = 0; i < ht_get_size(&scope->var_locations); i++) {
    const Hkey* key = ht_lookup_key(&scope->var_locations, i);
    const Hvalue* value = ht_lookup_byindex(&scope->var_locations, i);
    if (key != NULL && value != NULL) {
      for (int i = 0; i < level; i++) printf("  ");
      printf("  %s: ", *key);
      struct Object* object = &vm->variables[*value];
      if (object->type == T_FUNCTION) {
        print_recursive(vm, &object->value.func.scope, level + 1);
      }
      else {
        object_print(object);
        printf(",\n");
      }
    }
  }
  for (int i = 0; i < level; i++) printf("  ");
  printf("}\n");
  return 0;
}

static int base_print(struct VM_state* vm) {
  int arg_count = vm->stack_top - vm->stack_bp;
  for (int i = 0; i < arg_count; i++) {
    struct Object* obj = &vm->stack[vm->stack_bp + i];
    object_print(obj);
    printf("  ");
  }
  printf("\n");
  return 0;
}

static int base_print_global(struct VM_state* vm) {
  print_state(vm, &vm->global.scope);
  return 0;
}

static int base_print_recursive(struct VM_state* vm) {
  print_recursive(vm, &vm->global.scope, 0);
  return 0;
}
static struct Lib_def basemod_funcs[] = {
  {"print", base_print},
  {"print_global", base_print_global},
  {"print_recursive", base_print_recursive},
  {NULL, NULL},
};

extern struct Lib_def* libbase() {
  return basemod_funcs;
}