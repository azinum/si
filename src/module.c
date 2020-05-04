// module.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "vm.h"
#include "api.h"
#include "module.h"

int module_load(struct VM_state* vm, struct Module_def* module) {
  assert(module != NULL);
  for (; module->name; module++) {
    si_store_cfunc(vm, module->name, module->func);
  }
  return 0;
}