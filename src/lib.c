// lib.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "vm.h"
#include "api.h"
#include "lib.h"

int lib_load(struct VM_state* vm, struct Lib_def* lib) {
  for (; lib->name; lib++)
    si_store_cfunc(vm, lib->name, lib->func);
  return 0;
}
