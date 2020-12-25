// libtest.c
// gcc libtest.c -I/usr/include/si -Wall -shared -fPIC -o libtest.so -L/usr/lib/si -lsi
// gcc libtest.c -I/usr/local/include/si -Wall -shared -fPIC -o libtest.so -lsi

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
