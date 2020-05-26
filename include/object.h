// object.h

#ifndef _OBJECT_H
#define _OBJECT_H

#include "hash.h"
#include "config.h"
#include "token.h"

struct VM_state;
typedef double obj_number;

typedef int Instruction;

typedef int (*CFunction)(struct VM_state*);

struct Scope {
  unsigned int constants_count;
  struct Object* constants;
  Htable var_locations;
  struct Scope* parent;
};

struct Function {
  struct Scope scope;
  Instruction addr;
  int argc;
};

struct Object {
  union value {
    obj_number number;
    struct {
      char* data;
      int length;
    } str;
    struct Function func;
    CFunction cfunc;
  } value;
  int type;
};

struct Object token_to_object(struct Token token);

int func_init(struct Function* func);

int func_init_with_parent_scope(struct Function* func, struct Scope* parent);

int scope_init(struct Scope* scope, struct Scope* parent);

int scope_free(struct Scope* scope);

void object_printline(const struct Object* object);

void object_print(const struct Object* object);

extern int object_checktrue(const struct Object* object);

#endif
