// lib.h

#ifndef _LIB_H
#define _LIB_H

#include "object.h"

struct Lib_def {
  const char* name;
  CFunction func;
};

extern struct Lib_def* libbase();

int lib_load(struct VM_state* vm, struct Lib_def* lib);

#endif