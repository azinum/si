// compile.c
// node tree -> instruction sequence

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>

#include "error.h"
#include "config.h"
#include "str.h"
#include "hash.h"
#include "list.h"
#include "ast.h"
#include "vm.h"
#include "object.h"
#include "token.h"
#include "compile.h"

// Compile-time function state
struct Func_state {
  struct Function* func;
  struct Function* global;
  struct Function local;
  Htable args;
};

#define compile_error(fmt, ...) \
  error(COLOR_ERROR "compile-error: " COLOR_NONE fmt, ##__VA_ARGS__)

#define compile_warning(fmt, ...) \
  warn(COLOR_WARNING "compile-warning: " COLOR_NONE fmt, ##__VA_ARGS__)

#define compile_error2(token, fmt, ...) \
  error("%i:%i: " COLOR_ERROR "compile-error: " COLOR_NONE fmt, token->line, token->count, ##__VA_ARGS__)

#define UNRESOLVED_JUMP 0

static int instruction_add(struct VM_state* vm, Instruction instruction, unsigned int* ins_count);
static int func_state_init(struct Func_state* state, struct Function* global, int in_global_scope);
static void func_state_free(struct Func_state* state);
static const int* variable_lookup(struct VM_state* vm, struct Func_state* state, const char* identifier);
static int patchblock(struct VM_state* vm, int block_size);
static int compile_ifstatement(struct VM_state* vm, Ast* cond, Ast* block, struct Func_state* state, unsigned int* ins_count);
static int compile_whileloop(struct VM_state* vm, Ast* cond, Ast* block, struct Func_state* state, unsigned int* ins_count);
static int compile_function(struct VM_state* vm, struct Token* identifier, Ast* params, Ast* block, struct Func_state* state, unsigned int* ins_count);
static int compile(struct VM_state* vm, Ast* ast, struct Func_state* state, unsigned int* ins_count);
static int compile_pushk(struct VM_state* vm, struct Func_state* state, struct Token constant, unsigned int* ins_count);
static int compile_pushvar(struct VM_state* vm, struct Func_state* state, struct Token variable, unsigned int* ins_count);
static int compile_declvar(struct VM_state* vm, struct Func_state* state, struct Token variable);
static int get_variable_location(struct VM_state* vm, struct Func_state* state, struct Token variable, Instruction* location);
static int store_constant(struct VM_state* vm, struct Func_state* state, struct Token constant, Instruction* location);
static int store_variable(struct VM_state* vm, struct Func_state* state, struct Token variable, Instruction* location);
static int token_to_op(struct Token token);

int instruction_add(struct VM_state* vm, Instruction instruction, unsigned int* ins_count) {
  list_push(vm->program, vm->program_size, instruction);
  if (ins_count)
    (*ins_count)++;
  return NO_ERR;
}

int func_state_init(struct Func_state* state, struct Function* global, int in_global_scope) {
  assert(state != NULL);
  state->args = ht_create_empty();
  if (!in_global_scope)
    state->func = &state->local;
  else
    state->func = global;
  state->global = global;
  return NO_ERR;
}

void func_state_free(struct Func_state* state) {
  ht_free(&state->args);
}

// First check the local scope,
// then the parent scope,
// and finally the global scope
const int* variable_lookup(struct VM_state* vm, struct Func_state* state, const char* identifier) {
  struct Scope* scope = &state->func->scope;
  struct Scope* parent_scope = state->func->scope.parent;
  struct Scope* global_scope = &state->global->scope;
  assert(scope != NULL);
  assert(global_scope != NULL);
  const int* found = ht_lookup(&scope->var_locations, identifier);  // Find the location/index of the variable
  if (found)
    return found;

  if (parent_scope) {
    found = ht_lookup(&parent_scope->var_locations, identifier);
    if (found)
      return found;
  }
  found = ht_lookup(&global_scope->var_locations, identifier);
  return found;
}

// Update all goto/break statements in block
int patchblock(struct VM_state* vm, int block_size) {
  int end = vm->program_size - 1;
  for (int i = end - block_size; i < end; i++) {
    Instruction instruction = vm->program[i];
    if (instruction == I_JUMP || instruction == I_IF) {
      if (vm->program[i + 1] == UNRESOLVED_JUMP)  { // Fix unresolved jump
        vm->program[i + 1] = end - i;
      }
      i++;
    }
    else // Skip any other instruction that isn't a jump
      i += compile_get_ins_arg_count(instruction);
  }
  return NO_ERR;
}

int compile_pushk(struct VM_state* vm, struct Func_state* state, struct Token constant, unsigned int* ins_count) {
  Instruction location = -1;
  store_constant(vm, state, constant, &location);
  instruction_add(vm, I_PUSHK, ins_count);
  instruction_add(vm, location, ins_count);
  return NO_ERR;
}

int compile_pushvar(struct VM_state* vm, struct Func_state* state, struct Token variable, unsigned int* ins_count) {
  Instruction location = -1;
  Instruction push_instruction = I_PUSH_VAR;
  char* identifier = string_new_copy(variable.string, variable.length);
  const int* found = ht_lookup(&state->args, identifier);
  push_instruction = I_PUSH_ARG;
  if (!found) {
    found = variable_lookup(vm, state, identifier);
    push_instruction = I_PUSH_VAR;
  }
  string_free(identifier);
  if (!found) {
    compile_error2((&variable), "Undeclared identifier '%.*s'\n", variable.length, variable.string);
    return COMPILE_ERR;
  }
  location = *found;
  instruction_add(vm, push_instruction, ins_count);
  instruction_add(vm, location, ins_count);
  assert(location >= 0);
  return NO_ERR;
}

int compile_declvar(struct VM_state* vm, struct Func_state* state, struct Token variable) {
  Instruction location = -1;
  int err = store_variable(vm, state, variable, &location);
  if (err != NO_ERR) {
    compile_error2((&variable), "Identifier '%.*s' has already been declared\n", variable.length, variable.string);
    return err;
  }
  return NO_ERR;
}

int get_variable_location(struct VM_state* vm, struct Func_state* state, struct Token variable, Instruction* location) {
  struct Scope* scope = &state->func->scope;
  char* identifier = string_new_copy(variable.string, variable.length);
  const int* found = ht_lookup(&scope->var_locations, identifier);
  string_free(identifier);
  if (!found) {
    compile_error2((&variable), "No such variable '%.*s'\n", variable.length, variable.string);
    return COMPILE_ERR;
  }
  *location = *found;
  return NO_ERR;
}

int store_constant(struct VM_state* vm, struct Func_state* state, struct Token constant, Instruction* location) {
  assert(location != NULL);
  struct Scope* scope = &state->func->scope;
  *location = scope->constants_count;
  struct Object object = token_to_object(vm, constant);
  list_push(scope->constants, scope->constants_count, object);
  return NO_ERR;
}

int store_variable(struct VM_state* vm, struct Func_state* state, struct Token variable, Instruction* location) {
  assert(location != NULL);
  struct Scope* scope = &state->func->scope;
  char* identifier = string_new_copy(variable.string, variable.length);
  if (ht_element_exists(&scope->var_locations, identifier)) {
    string_free(identifier);
    return ERR;
  }
  else {
    struct Object object = token_to_object(vm, variable);
    *location = vm->variable_count;
    ht_insert_element(&scope->var_locations, identifier, *location);
    list_push(vm->variables, vm->variable_count, object);
    assert(ht_element_exists(&scope->var_locations, identifier) != 0);
    string_free(identifier);
  }
  return NO_ERR;
}

#define OP_CASE(OP) case T_##OP: return I_##OP

int token_to_op(struct Token token) {
  switch (token.type) {
    OP_CASE(ADD);
    OP_CASE(SUB);
    OP_CASE(MULT);
    OP_CASE(DIV);
    OP_CASE(LT);
    OP_CASE(GT);
    OP_CASE(EQ);
    OP_CASE(LEQ);
    OP_CASE(GEQ);
    OP_CASE(NEQ);
    OP_CASE(MOD);
    OP_CASE(BAND);
    OP_CASE(BOR);
    OP_CASE(BXOR);
    OP_CASE(LEFTSHIFT);
    OP_CASE(RIGHTSHIFT);
    OP_CASE(AND);
    OP_CASE(OR);
    OP_CASE(NOT);
    default:
      break;
  }
  return I_UNKNOWN;
}

// Generated code:
// COND ...
// i_if, jump,
//   BLOCK ...
int compile_ifstatement(struct VM_state* vm, Ast* cond, Ast* block, struct Func_state* state, unsigned int* ins_count) {
  compile(vm, cond, state, ins_count);
  unsigned int block_size = 0;
  instruction_add(vm, I_IF, &block_size);
  instruction_add(vm, UNRESOLVED_JUMP, ins_count);
  Instruction jump_index = vm->program_size - 1;
  compile(vm, block, state, &block_size);
  list_assign(vm->program, vm->program_size, jump_index, block_size);
  *ins_count += block_size;
  return NO_ERR;
}

// Generated code:
// COND ...
// i_while, jump,
//   BLOCK ...
// jump_back (to COND)
int compile_whileloop(struct VM_state* vm, Ast* cond, Ast* block, struct Func_state* state, unsigned int* ins_count) {
  unsigned int cond_size = 0;
  unsigned int block_size = 0;
  compile(vm, cond, state, &cond_size);
  instruction_add(vm, I_WHILE, &block_size);
  instruction_add(vm, UNRESOLVED_JUMP, ins_count);
  int jump_index = vm->program_size - 1;
  compile(vm, block, state, &block_size);
  instruction_add(vm, I_JUMP, &block_size);
  instruction_add(vm, UNRESOLVED_JUMP, &block_size);
  int jumpback_index = vm->program_size - 1;
  list_assign(vm->program, vm->program_size, jump_index, block_size);
  list_assign(vm->program, vm->program_size, jumpback_index, -(block_size + cond_size));
  assert(vm->program[jump_index] != 0 && vm->program[jump_index] != 0);
  *ins_count += block_size + cond_size;
  patchblock(vm, block_size); // Patch up all unresolved jumps in this block
  return NO_ERR;
}

// T_FUNC_DEF
// identifier
//  \--> ( parameter list )
// T_BLOCK
//  \--> { BLOCK }
int compile_function(struct VM_state* vm, struct Token* identifier, Ast* params, Ast* block, struct Func_state* state, unsigned int* ins_count) {
  unsigned int block_size = 0;  // The whole function block
  instruction_add(vm, I_JUMP, &block_size);
  instruction_add(vm, UNRESOLVED_JUMP, &block_size);
  Instruction func_addr = vm->program_size;
  struct Func_state func_state;
  func_state_init(&func_state, state->global, 0);
  func_init_with_parent_scope(func_state.func, &state->func->scope);
  func_state.func->addr = func_addr;
  Instruction location = -1;
  int status = store_variable(vm, state, *identifier, &location);
  if (status != NO_ERR) {
    compile_error2(identifier, "Identifier '%.*s' has already been declared\n", identifier->length, identifier->string);
    return vm->status = status;
  }
  int arg_count = ast_child_count(params);
  for (int i = 0; i < arg_count; i++) {
    const struct Token* value = ast_get_node_value(params, i);
    assert(value != NULL);
    char* arg_key = string_new_copy(value->string, value->length);
    if (ht_lookup(&func_state.args, arg_key)) {
      compile_error2(value, "Parameter '%s' has already been identified\n", arg_key);
      string_free(arg_key);
      func_state_free(&func_state);
      return vm->status = COMPILE_ERR;
    }
    ht_insert_element(&func_state.args, arg_key, i);
    string_free(arg_key);
  }
  func_state.func->argc = arg_count;
  compile(vm, block, &func_state, &block_size);  // Compile the function body
  instruction_add(vm, I_RETURN, &block_size);
  patchblock(vm, block_size); // Fix the unresolved jump (skip the function block)
  struct Object* func = &vm->variables[location];
  func->type = T_FUNCTION;
  func->value.func = *func_state.func; // Apply function compile state to the 'real' function
  *ins_count += block_size;
  func_state_free(&func_state);
  return NO_ERR;
}

int compile(struct VM_state* vm, Ast* ast, struct Func_state* state, unsigned int* ins_count) {
  assert(ins_count != NULL);
  assert(vm != NULL);
  assert(state != NULL);
  struct Token* token = NULL;
  for (int i = 0; i < ast_child_count(ast); i++) {
    token = ast_get_node_value(ast, i);
    if (token) {
      switch (token->type) {
        case T_STRING:
        case T_NUMBER:
          compile_pushk(vm, state, *token, ins_count);
          break;

        case T_IDENTIFIER: {
          int status = compile_pushvar(vm, state, *token, ins_count);
          if (status != NO_ERR)
            return vm->status = status;
          break;
        }

        // decl
        // identifier
        // \--> expr
        case T_DECL: {
          struct Token* identifier = ast_get_node_value(ast, ++i);
          assert(identifier != NULL);
          int status = compile_declvar(vm, state, *identifier);
          if (status != NO_ERR)
            return vm->status = status;
          Ast expr_branch = ast_get_node_at(ast, i);
          assert(ast_child_count(&expr_branch) > 0);
          compile(vm, &expr_branch, state, ins_count);  // Compile the right-hand side expression
          Instruction location = -1;
          get_variable_location(vm, state, *identifier, &location);
          assert(location >= 0);
          instruction_add(vm, I_ASSIGN, ins_count);
          instruction_add(vm, location, ins_count);
          break;
        }

        // { assign, identifier }
        case T_ASSIGN: {
          struct Token* identifier_token = ast_get_node_value(ast, ++i);
          assert(identifier_token != NULL);
          char* identifier = string_new_copy(identifier_token->string, identifier_token->length);
          Instruction location = -1;
          const int* found = variable_lookup(vm, state, identifier);
          string_free(identifier);
          if (!found) {
            compile_error2(identifier_token, "%s\n", "No such variable");
            return vm->status = COMPILE_ERR;
          }
          location = *found;
          assert(location >= 0);
          instruction_add(vm, I_ASSIGN, ins_count);
          instruction_add(vm, location, ins_count);
          break;
        }

        case T_RETURN:
          instruction_add(vm, I_RETURN, ins_count);
          break;

        case T_IF: {
          Ast cond = ast_get_node_at(ast, i);
          Ast block = ast_get_node_at(ast, ++i);
          assert(cond != NULL);
          assert(block != NULL);
          compile_ifstatement(vm, &cond, &block, state, ins_count);
          break;
        }

        case T_WHILE: {
          Ast cond = ast_get_node_at(ast, i);
          Ast block = ast_get_node_at(ast, ++i);
          assert(cond != NULL);
          assert(block != NULL);
          compile_whileloop(vm, &cond, &block, state, ins_count);
          break;
        }

        case T_BREAK:
          instruction_add(vm, I_JUMP, ins_count);
          instruction_add(vm, UNRESOLVED_JUMP, ins_count);
          break;

        case T_FUNC_DEF: {
          struct Token* identifier = ast_get_node_value(ast, ++i);
          Ast params = ast_get_node_at(ast, i);
          Ast block = ast_get_node_at(ast, ++i);
          assert(block != NULL);
          int status = compile_function(vm, identifier, &params, &block, state, ins_count);
          if (status != NO_ERR)
            return vm->status = status;
          break;
        }

        case T_CALL: {
          Ast args_branch = ast_get_node_at(ast, i);
          compile(vm, &args_branch, state, ins_count);
          const struct Token* num_args_token = ast_get_node_value(ast, ++i);
          int num_args = num_args_token->value.integer;
          instruction_add(vm, I_CALL, ins_count);
          instruction_add(vm, num_args, ins_count);
          break;
        }

        case T_LOAD: {
          ++i;
          struct Token* path_token = ast_get_node_value(ast, ++i);
          assert(path_token != NULL);
          char path[PATH_LENGTH_MAX] = {0};
          snprintf(path, PATH_LENGTH_MAX, "./%.*s.so", path_token->length, path_token->string);
          void* lib_handle = dlopen(path, RTLD_LAZY);
          if (!lib_handle) {
            compile_error2(path_token, "Failed to open library '%s'; %s\n", path, dlerror());
            return vm->status = COMPILE_ERR;
          }
          CFunction init = dlsym(lib_handle, "init");
          if (!init) {
            compile_error2(path_token, "Failed to find symbol 'init'\n");
            return vm->status = COMPILE_ERR;
          }
          init(vm);
          break;
        }

        default: {
          int op = token_to_op(*token);
          if (op != I_UNKNOWN) {
            instruction_add(vm, op, ins_count);
            break;
          }
          assert(0);
          compile_error2(token, "%s\n", "Invalid instruction");
          return vm->status = COMPILE_ERR;
        }
      }
    }
  }
  return NO_ERR;
}

int compile_from_tree(struct VM_state* vm, Ast* ast) {
  assert(ast != NULL);
  assert(vm != NULL);
  if (ast_is_empty(*ast))
    return NO_ERR;
  struct Func_state global_state;
  func_state_init(&global_state, &vm->global, 1);
  global_state.global = &vm->global;
  unsigned int ins_count = 0;
  compile(vm, ast, &global_state, &ins_count);
  instruction_add(vm, I_RETURN, NULL);
  func_state_free(&global_state);
  return vm->status;
}

unsigned int compile_get_ins_arg_count(Instruction instruction) {
  switch (instruction) {
    case I_ASSIGN:
    case I_PUSHK:
    case I_PUSH_VAR:
    case I_IF:
    case I_WHILE:
    case I_JUMP:
    case I_PUSH_ARG:
    case I_CALL:
      return 1;
    default:
      return 0;
  }
}
