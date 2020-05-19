// api.h

#ifndef _API_H
#define _API_H

#include "error.h"

#define si_error(fmt, ...) \
  error("%s: " COLOR_ERROR "api-error: " COLOR_NONE fmt, __FUNCTION__,  ##__VA_ARGS__)

int si_store_object(struct VM_state* vm, struct Scope* scope, const char* name, struct Object object);

int si_store_number(struct VM_state* vm, const char* name, double number);

int si_store_cfunc(struct VM_state* vm, const char* name, CFunction cfunc);

#endif
