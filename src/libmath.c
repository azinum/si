// libmath.c

#include <math.h>
#include <stdlib.h>

#include "api.h"
#include "lib.h"

int fib(int n) {
  if (n <= 2)
    return n;
  return fib(n - 2) + fib(n - 1);
}

// TODO(lucas): Automate this process of interfacing the language with C functions.
static int math_fib(struct VM_state* vm) {
  if ((vm->stack_top - vm->stack_bp) <= 0) {
    si_error("Missing argument\n");
    return 0;
  }
  const struct Object obj = vm->stack[vm->stack_bp + 0];  // TODO(lucas): Make an arg fetching function, so that you don't have to do this every time you need to get args
  if (obj.type != T_NUMBER) {
    si_error("Expected a number type\n");
    return 0;
  }
  int result = fib((int)obj.value.number);
  si_push_number(vm, result);
  return 1; // We are returning a value!
}

static struct Lib_def libmath_funcs[] = {
  {"fib", math_fib},
  {NULL, NULL},
};

extern struct Lib_def* libmath() {
  return libmath_funcs;
}
