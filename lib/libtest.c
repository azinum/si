// libtest.c
// clang libtest.c -Iinclude -Wall -shared -fPIC -o libtest.so -Lbuild/shared/si

#include <si.h>
#include <stdio.h>
#include <stdlib.h>

static int test_test(struct VM_state* vm) {
  printf("%s:%s\n", __FILE__, __FUNCTION__);
  return 0;
}

static struct Lib_def libtest_funcs[] = {
  {"test", test_test},
  {NULL, NULL}
};

extern int init(struct VM_state* vm) {
  lib_load(vm, libtest_funcs);
  return 0;
}
