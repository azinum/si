// api.h

#ifndef _API_H
#define _API_H

int si_store_object(struct VM_state* vm, struct Scope* scope, const char* name, struct Object object);

int si_store_number(struct VM_state* vm, const char* name, double number);

int si_store_cfunc(struct VM_state* vm, const char* name, CFunction cfunc);

#endif