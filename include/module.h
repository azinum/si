// module.h

#ifndef _MODULE_H
#define _MODULE_H

#include "object.h"

struct Module_def {
  const char* name;
  CFunction func;
};

extern struct Module_def* basemod();

int module_load(struct VM_state* vm, struct Module_def* module);

#endif