// vm.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "str.h"
#include "mem.h"
#include "list.h"
#include "strarr.h"
#include "ast.h"
#include "parser.h"
#include "compile.h"
#include "api.h"
#include "lib.h"
#include "stack.h"
#include "vm.h"

static const char* ins_descriptions[INSTRUCTION_COUNT] = {
  "unknown",
  "add",
  "sub",
  "mult",
  "div",
  "less_than",
  "greater_than",
  "equal",
  "less_equal",
  "greater_equal",
  "not_equal",
  "mod",
  "bitwise_and",
  "bitwise_or",
  "bitwise_xor",
  "leftshift",
  "rightshift",
  "and",
  "or",
  "not",

  "assign",
  "pushk",
  "pop",
  "pushvar",
  "return",
  "if",
  "while",
  "jump",
  "call",
  "push_arg",

  "exit",
};

#define vmdispatch(instruction) switch (instruction)
#define vmcase(c) case c:
#define vmbreak break
#define vmfetch() { \
  i = *(ip++); \
}
#define vmjump(n) i = *(ip += n)
// TODO: Fix goto!
#define vmgoto(n) { \
  i = *(ip += (ip - (vm->program + n))); \
}

#define OP_ARITH_CAST(OP, CAST) { \
  if (vm->stack_top > 1) { \
    struct Object* left = stack_get(vm, 1); \
    const struct Object* right = stack_gettop(vm); \
    assert((left != NULL) && (right != NULL)); \
    if (OP_SAMETYPE((*left), (*right), T_NUMBER)) { \
      left->value.number = ((CAST)left->value.number) OP ((CAST)right->value.number); \
      stack_pop(vm); \
    } \
    else \
      vmerror("Invalid types in arithmetic operation\n"); \
  } \
} \

#define OP_ARITH(OP) OP_ARITH_CAST(OP, obj_number)

#define OP_INT_ARITH(OP) OP_ARITH_CAST(OP, int)

#define UNOP_ARITH(UOP) { \
  if (vm->stack_top > 0) { \
    struct Object* top = stack_gettop(vm); \
    assert(top != NULL); \
    if (top->type == T_NUMBER) { \
      top->value.number = UOP(top->value.number); \
    } \
    else \
      vmerror("Invalid types in unary arithmetic operation\n"); \
  } \
} \

#define OP_SAMETYPE(LEFT, RIGHT, TYPE) (LEFT.type == TYPE && RIGHT.type == TYPE)

inline struct Object* get_variable(struct VM_state* vm, struct Scope* scope, int var);
inline int equal_types(const struct Object* a, const struct Object* b);
static int execute(struct VM_state* vm, struct Function* func);
static int disasm(struct VM_state* vm, FILE* file);
static int free_variables(struct VM_state* vm);

struct Object* get_variable(struct VM_state* vm, struct Scope* scope, int var) {
  assert(var >= 0 && vm->variable_count > var);
  return &vm->variables[var];
}

int equal_types(const struct Object* a, const struct Object* b) {
  assert(a != NULL);
  assert(b != NULL);
  return a->type == b->type;
}


int execute(struct VM_state* vm, struct Function* func) {
#if defined(USE_JUMPTABLE)
#include "jumptable.h"
#endif
  Instruction* ip = &vm->program[func->addr];
  Instruction i = I_RETURN;
  int stack_bp = vm->stack_bp;
  for (;;) {
    vmfetch();
    vmdispatch(i) {
      vmcase(I_ASSIGN) {
        int var_location = *(ip++);
        struct Object* variable = get_variable(vm, &func->scope, var_location);
        const struct Object* top = stack_gettop(vm);
        if (!top) {
          // TEMP
          vmbreak;
        }
        if (top->type == T_FUNCTION) {
          vmerror("Can't assign function to variable\n");
          return RUNTIME_ERR;
        }
        if (variable->type == T_FUNCTION) {
          vmerror("Can't modify function\n");
          return RUNTIME_ERR;
        }
        *variable = *top;
        stack_pop(vm);
        vmbreak;
      }

      vmcase(I_PUSHK) {
        int constant = *(ip++);
        stack_pushk(vm, &func->scope, constant);
        vmbreak;
      }

      vmcase(I_POP)
        stack_pop(vm);
        vmbreak;

      vmcase(I_PUSH_VAR) {
        int variable = *(ip++);
        stack_pushvar(vm, &func->scope, variable);
        vmbreak;
      }

      vmcase(I_RETURN)
        goto done_exec;

      // Input: { COND if jmp BLOCK }
      // jump if condition is false (skip if-block)
      // continue if true (skip jump address)
      vmcase(I_IF) {
        const struct Object* top = stack_gettop(vm);
        if (object_checktrue(top)) {
          stack_pop(vm);
          ip++; // Skip jump
          vmbreak;  // Enter if-block
        }
        stack_pop(vm);
        int jump = *(ip);
        assert(jump >= 0 && jump < vm->program_size);
        vmjump(jump);
        vmbreak;
      }

      // Input: { COND while jmp BLOCK jmp_back }
      // Continue if condition is true (skip jump)
      // Jump if condition is false (next instruction is jump)
      vmcase(I_WHILE) {
        const struct Object* top = stack_gettop(vm);
        if (object_checktrue(top)) {
          stack_pop(vm);
          ip++; // Skip jump
          vmbreak;  // Enter while-block
        }
        stack_pop(vm);
        int jump = *(ip);
        assert(jump != 0);
        vmjump(jump);
        vmbreak;
      }

      vmcase(I_JUMP) {
        int jump = *(ip);
        vmjump(jump);
        vmbreak;
      }

      // func, arg1, arg2, ..., CALL, arg_count
      vmcase(I_CALL) {
        int arg_count = *(ip++);
        int bp = vm->stack_top - arg_count;
        vm->stack_bp = bp;
        const struct Object* obj = stack_get(vm, arg_count);
        if (obj->type == T_CFUNCTION) {
          int result = obj->value.cfunc(vm);
          if (result == 1) {
            struct Object* top = stack_gettop(vm);
            vm->stack[bp - 1] = *top; // NOTE(lucas): The return value lies on the top of the stack after a C function call - might change later
            vm->stack_top = bp;
          }
          else {
            vm->stack_top = bp;
            stack_pop(vm);
          }
          vmbreak;
        }
        if (obj->type != T_FUNCTION) {
          vmerror("Attempted to call a non-function value\n");
          return RUNTIME_ERR;
        }
        struct Function function = obj->value.func;
        if (function.argc != arg_count) {
          vmerror("Invalid number of arguments (should be: %i)\n", function.argc);
          return RUNTIME_ERR;
        }
        Instruction* old_ip = ip;
        execute(vm, &function);
        vm->stack[bp - 1] = *stack_gettop(vm);
        vm->stack_top = bp;
        *ip = *old_ip;
        vmbreak;
      }

      vmcase(I_PUSH_ARG) {
        int arg_location = *(ip++);
        struct Object arg = vm->stack[stack_bp + arg_location];
        stack_push(vm, arg);
        vmbreak;
      }

      vmcase(I_ADD)
        OP_ARITH(+);
        vmbreak;

      vmcase(I_SUB)
        OP_ARITH(-);
        vmbreak;

      vmcase(I_MULT)
        OP_ARITH(*);
        vmbreak;

      vmcase(I_DIV)
        OP_ARITH(/);
        vmbreak;

      vmcase(I_LT)
        OP_ARITH(<);
        vmbreak;

      vmcase(I_GT)
        OP_ARITH(>);
        vmbreak;

      vmcase(I_EQ)
        OP_ARITH(==);
        vmbreak;

      vmcase(I_LEQ)
        OP_ARITH(<=);
        vmbreak;

      vmcase(I_GEQ)
        OP_ARITH(>=);
        vmbreak;

      vmcase(I_NEQ)
        OP_ARITH(!=);
        vmbreak;

      vmcase(I_MOD)
        OP_INT_ARITH(%);
        vmbreak;

      vmcase(I_BAND)
        OP_INT_ARITH(&);
        vmbreak;

      vmcase(I_BOR)
        OP_INT_ARITH(|);
        vmbreak;

      vmcase(I_BXOR)
        OP_INT_ARITH(^);
        vmbreak;

      vmcase(I_LEFTSHIFT)
        OP_INT_ARITH(<<);
        vmbreak;

      vmcase(I_RIGHTSHIFT)
        OP_INT_ARITH(>>);
        vmbreak;

      vmcase(I_AND)
        OP_ARITH(&&);
        vmbreak;

      vmcase(I_OR)
        OP_ARITH(||);
        vmbreak;

      vmcase(I_NOT)
        UNOP_ARITH(!);
        vmbreak;

      vmcase(I_UNKNOWN)
        assert(0);
        vmbreak;

      vmcase(I_EXIT)
        goto done_exec;
    }
  }
done_exec:
  return NO_ERR;
}

int disasm(struct VM_state* vm, FILE* file) {
  assert(file != NULL);
  fprintf(file, "[%i instructions, %i variables, %i constants]\n", vm->program_size, vm->variable_count, vm->global.scope.constants_count);
  for (int i = 0; i < vm->program_size; i++) {
    Instruction instruction = vm->program[i];
    unsigned int arg_count = compile_get_ins_arg_count(instruction);
    if (arg_count > 0) {
      fprintf(file, "%.4i %-14s%i\n", i, ins_descriptions[instruction], vm->program[i + 1]);
      i += arg_count;
    }
    else {
      fprintf(file, "%.4i %s\n", i, ins_descriptions[instruction]);
    }
  }
  return NO_ERR;
}


int free_variables(struct VM_state* vm) {
  for (int i = 0; i < vm->variable_count; i++) {
    struct Object* obj = &vm->variables[i];
    if (obj) {
      switch (obj->type) {
        case T_FUNCTION:
          scope_free(&obj->value.func.scope);
          break;

        case T_STRING: {
          if (obj->value.str.data != NULL) {
            // NOTE(lucas): Strings are stored in the buffer array (vm->buffers)
            // string_free(obj->value.str.data);
            // obj->value.str.length = 0;
          }
          break;
        }
        default:
          break;
      }
    }
  }
  list_free(vm->variables, vm->variable_count);
  vm->variable_count = 0;
  return NO_ERR;
}

int vm_init(struct VM_state* vm) {
  assert(vm != NULL);
  func_init(&vm->global);
  vm->variables = NULL;
  vm->variable_count = 0;
  strarr_init(&vm->buffers);
  vm->stack_top = 0;
  vm->stack_bp = 0;
  vm->status = NO_ERR;
  vm->program = NULL;
  vm->program_size = 0;
  vm->prev_ip = 0;
  vm->heap_allocated = 0;
  lib_load(vm, libbase());
  lib_load(vm, libmath());
  return vm->status;
}

struct VM_state* vm_state_new() {
  struct VM_state* vm = mmalloc(sizeof(struct VM_state));
  if (!vm) {
    vmerror("Failed to initialize VM state\n");
    return NULL;
  }
  vm_init(vm);
  vm->heap_allocated = 1;
  return vm;
}

int vm_exec(struct VM_state* vm, const char* filename, char* input, struct Str_arr* str_arr) {
  assert(input != NULL);
  assert(vm != NULL);
  Ast ast = ast_create();
  if (parser_parse(input, str_arr, filename, &ast) == NO_ERR) {
#if 0
    compile_from_tree(vm, &ast);
    if (vm->status == NO_ERR) {
      if (vm->prev_ip != vm->program_size) {  // Has program changed since last vm execution? 
        execute(vm, &vm->global);
        stack_print_top(vm);
        stack_reset(vm);
        list_shrink(vm->program, vm->program_size, 1);  // If so, remove the exit instruction
        vm->prev_ip = vm->program_size;
      }
    }
#else
    // ast_print(ast);
#endif
    vm->global.addr = vm->program_size; // We're in interactive mode, move the start posiiton to the last instruction
  }
  ast_free(&ast);
  return vm->status;
}

int vm_disasm(struct VM_state* vm, const char* output_file) {
  FILE* file = fopen(output_file, "w");
  if (!file) {
    error("%s: Failed to open file '%s'", __FUNCTION__, output_file);
    return ERR;
  }
  disasm(vm, file);
  fclose(file);
  return NO_ERR;
}

void vm_state_free(struct VM_state* vm) {
  assert(vm != NULL);
  scope_free(&vm->global.scope);
  free_variables(vm);
  strarr_free(&vm->buffers);
  vm->stack_top = 0;
  vm->status = 0;
  list_free(vm->program, vm->program_size);
  vm->program_size = 0;
  vm->prev_ip = 0;
  if (vm->heap_allocated)
    mfree(vm, sizeof(struct VM_state));
  vm->heap_allocated = 0;
  if (memory_alloc_count() != 0 || memory_alloc_total() != 0) {
    si_error("Memory leak!\n");
    memory_print_info();
  }
  assert(!memory_alloc_count() && !memory_alloc_total());
}
