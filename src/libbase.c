// libbase.c

#include <stdlib.h>
#include <stdio.h>

#include "si.h"

int print_state(struct VM_state* vm, struct Scope* scope, int level) {
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
        print_state(vm, &object->value.func.scope, level + 1);
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
  int arg_count = si_get_argc(vm);
  if (arg_count <= 0) {
    si_error("Missing arguments\n");
    return 0;
  }
  for (int i = 0; i < arg_count; i++) {
    struct Object* obj = &vm->stack[vm->stack_bp + i];
    object_print(obj);
    printf(" ");
  }
  printf("\n");
  return 0;
}

static int base_printf(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count <= 0) {
    si_error("Missing arguments\n");
    return 0;
  }
  struct Object* format = &vm->stack[vm->stack_bp];
  if (format->type != T_STRING) {
    si_error("Expected argument of type string\n");
    return 0;
  }
  int num_args_used = 0;
  char* copy = string_new_copy(format->value.str.data, format->value.str.length);
  for (int i = 0; i < format->value.str.length; i++) {
    if (copy[i] == '\\') {  // This is an escape sequence
      switch (copy[++i]) {
        case 'a':
          printf("\a");
          break;
        case 'n':
          printf("\n");
          break;
        case 'r':
          printf("\r");
          break;
        case 't':
          printf("\t");
          break;
        default:
          printf("%c", copy[i]);
          break;
      }
      continue;
    }
    if (copy[i] == '%') {
      struct Object* arg = NULL;
      if (num_args_used + 1 < arg_count) {
        arg = &vm->stack[vm->stack_bp + num_args_used + 1];
        num_args_used++;
      }
      else {
        si_error("More format '%%' than data arguments\n");
        goto done;
      }
      switch (copy[i + 1]) {
        case '!': {
          if (arg->type == T_STRING)
            printf("\x1B[%.*sm", arg->value.str.length, arg->value.str.data);
          else if (arg->type == T_NUMBER)
            printf("\x1B[%im", (int)arg->value.number);
          i++;
          break;
        }
        default:
          object_print_raw(arg);
          break;
      }
      continue;
    }
    printf("%c", copy[i]);
  }
done:
  string_nfree(copy, format->value.str.length);
  return 0;
}

static int base_rand(struct VM_state* vm) {
  double result  = rand();
  si_push_number(vm, result);
  return 1;
}

static int base_print_state(struct VM_state* vm) {
  print_state(vm, &vm->global.scope, 0);
  return 0;
}

static int base_print_mem(struct VM_state* vm) {
  memory_print_info();
  return 0;
}

static int base_assert(struct VM_state* vm) {
  struct Object* obj = &vm->stack[vm->stack_bp];
  if (!object_checktrue(obj)) {
    printf("Assertion failed!\n");
    return 0;
  }
  return 0;
}

static struct Lib_def baselib_funcs[] = {
  {"print", base_print},
  {"printf", base_printf},
  {"rand", base_rand},
  {"print_state", base_print_state},
  {"print_mem", base_print_mem},
  {"assert", base_assert},
  {NULL, NULL},
};

extern struct Lib_def* libbase() {
  return baselib_funcs;
}
