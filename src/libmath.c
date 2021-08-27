// libmath.c

#include <math.h>
#include <stdlib.h>

#include "common.h"
#include "api.h"

int fib(int n) {
  if (n <= 2)
    return n;
  return fib(n - 2) + fib(n - 1);
}

static int math_fib(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count <= 0) {
    si_error("Missing arguments\n");
    return 0;
  }
  const struct Object* obj = si_get_arg(vm, 0);
  if (obj->type != T_NUMBER) {
    si_error("Expected a number type\n");
    return 0;
  }
  int result = fib((int)obj->value.number);
  si_push_number(vm, result);
  return 1; // We are returning a value!
}

static int math_sin(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count <= 0) {
    si_error("Missing arguments\n");
    return 0;
  }
  const struct Object* obj = si_get_arg(vm, 0);
  if (obj->type != T_NUMBER) {
    si_error("Expected a number type\n");
    return 0;
  }
  obj_number result = sin(obj->value.number);
  si_push_number(vm, result);
  return 1;
}

static int math_cos(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count <= 0) {
    si_error("Missing arguments\n");
    return 0;
  }
  const struct Object* obj = si_get_arg(vm, 0);
  if (obj->type != T_NUMBER) {
    si_error("Expected a number type\n");
    return 0;
  }
  obj_number result = cos(obj->value.number);
  si_push_number(vm, result);
  return 1;
}

static int math_log(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count <= 0) {
    si_error("Missing arguments\n");
    return 0;
  }
  const struct Object* obj = si_get_arg(vm, 0);
  if (obj->type != T_NUMBER) {
    si_error("Expected a number type\n");
    return 0;
  }
  obj_number result = log(obj->value.number);
  si_push_number(vm, result);
  return 1;
}

static int math_abs(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count <= 0) {
    si_error("Missing arguments\n");
    return 0;
  }
  const struct Object* obj = si_get_arg(vm, 0);
  if (obj->type != T_NUMBER) {
    si_error("Expected a number type\n");
    return 0;
  }
  obj_number result = abs(obj->value.number);
  si_push_number(vm, result);
  return 1;
}

static int math_sqrt(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count <= 0) {
    si_error("Missing arguments\n");
    return 0;
  }
  const struct Object* obj = si_get_arg(vm, 0);
  if (obj->type != T_NUMBER) {
    si_error("Expected a number type\n");
    return 0;
  }
  obj_number result = sqrt(obj->value.number);
  si_push_number(vm, result);
  return 1;
}

static int math_rad(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count <= 0) {
    si_error("Missing arguments\n");
    return 0;
  }
  const struct Object* obj = si_get_arg(vm, 0);
  if (obj->type != T_NUMBER) {
    si_error("Expected a number type\n");
    return 0;
  }
  obj_number result = obj->value.number * (PI32 / 180.0f);
  si_push_number(vm, result);
  return 1;
}

static struct Lib_def libmath_funcs[] = {
  {"fib", math_fib},
  {"sin", math_sin},
  {"cos", math_cos},
  {"log", math_log},
  {"abs", math_abs},
  {"sqrt", math_sqrt},
  {"rad", math_rad},
  {NULL, NULL},
};

extern struct Lib_def* libmath() {
  return libmath_funcs;
}
