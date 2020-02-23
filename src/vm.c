// vm.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "config.h"
#include "list.h"
#include "ast.h"
#include "hash.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

#define vmerror(fmt, ...) \
	error(COLOR_ERROR "vm-error: " COLOR_NONE fmt, ##__VA_ARGS__)

#define OP_ARITH(OP) { \
	if (vm->stack_top > 1) { \
	  struct Object* left = stack_get(vm, 1); \
	  const struct Object* right = stack_gettop(vm); \
	  assert((left != NULL) && (right != NULL)); \
	  if (OP_SAMETYPE((*left), (*right), T_NUMBER)) { \
	    left->value.number = left->value.number OP right->value.number; \
	    vm->stack_top--; \
	  } \
	  else vmerror("Invalid types in binary arithmetic operation\n"); \
	} \
} \

#define UNOP_ARITH(UOP) { \
	if (vm->stack_top > 0) { \
	  struct Object* top = stack_gettop(vm); \
	  assert(top != NULL); \
	  if (top->type == T_NUMBER) { \
	    top->value.number = UOP(top->value.number); \
	  } \
	  else vmerror("Invalid types in unary arithmetic operation\n"); \
	} \
} \

#define OP_SAMETYPE(LEFT, RIGHT, TYPE) (LEFT.type == TYPE && RIGHT.type == TYPE)

inline int stack_push(struct VM_state* vm, struct Object object);
inline int stack_pushk(struct VM_state* vm, struct Scope* scope, int constant);
inline int stack_pushvar(struct VM_state* vm, struct Scope* scope, int var);
inline int stack_reset(struct VM_state* vm);
inline int stack_print_top(struct VM_state* vm);
inline struct Object* stack_gettop(struct VM_state* vm);
inline struct Object* stack_get(struct VM_state* vm, int offset);

static int vm_init(struct VM_state* vm);
static int vm_dispatch(struct VM_state* vm, struct Function* func);

int stack_push(struct VM_state* vm, struct Object object) {
	if (vm->stack_top >= STACK_SIZE) {
		vmerror("Stack overflow!\n");
		return vm->status = STACK_ERR;
	}
	vm->stack[vm->stack_top++] = object;
	return NO_ERR;
}

int stack_pushk(struct VM_state* vm, struct Scope* scope, int constant) {
	assert(scope->constants_count > constant);
	struct Object object = scope->constants[constant];
	return stack_push(vm, object);
}

int stack_pushvar(struct VM_state* vm, struct Scope* scope, int var) {
	return NO_ERR;
}

int stack_reset(struct VM_state* vm) {
	vm->stack_top = 0;
	return NO_ERR;
}

int stack_print_top(struct VM_state* vm) {
	struct Object* top = stack_gettop(vm);
	if (top)
		object_print(*top);
	else
		printf("{}\n");
	return NO_ERR;
}

struct Object* stack_gettop(struct VM_state* vm) {
	return stack_get(vm, 0);
}

struct Object* stack_get(struct VM_state* vm, int offset) {
	int index = vm->stack_top - (offset + 1);	// offset 0 is top of stack
	if (index >= 0) return &vm->stack[index];
	return NULL;
}

int vm_init(struct VM_state* vm) {
	assert(vm != NULL);
	scope_init(&vm->global.scope, NULL);
	vm->variables = NULL;
	vm->stack_top = 0;
	vm->status = NO_ERR;
	vm->program = NULL;
	vm->program_size = 0;
	return vm->status;
}

int vm_dispatch(struct VM_state* vm, struct Function* func) {
	// Normal dispatch for now
	// TODO: convert this to a jumptable
	int instruction = I_UNKNOWN;
	for (int i = func->addr; i < vm->program_size; i++) {
		instruction = vm->program[i];
		switch (instruction) {
			case I_PUSHK: {
				int constant = vm->program[++i];
				stack_pushk(vm, &func->scope, constant);
			}
				break;

			case I_ADD:
				OP_ARITH(+);
				break;

			case I_SUB:
				OP_ARITH(-);
				break;

			case I_MULT:
				OP_ARITH(*);
				break;

			case I_DIV:
				OP_ARITH(/);
				break;

			case I_LT:
				OP_ARITH(<);
				break;

			case I_GT:
				OP_ARITH(>);
				break;

			case I_EQ:
				OP_ARITH(==);
				break;

			case I_LEQ:
				OP_ARITH(<=);
				break;

			case I_GEQ:
				OP_ARITH(>=);
				break;

			case I_NEQ:
				OP_ARITH(!=);
				break;

			case I_NOT:
				UNOP_ARITH(!);
				break;

			default:
				break;
		}
	}
	stack_print_top(vm);
	return NO_ERR;
}

int vm_exec(char* input) {
	assert(input != NULL);
	struct VM_state vm = {};
	vm_init(&vm);

	Ast ast = ast_create();
	if (parser_parse(input, &ast) == NO_ERR) {
		if (compile_from_tree(&vm, &ast) == NO_ERR)
			vm_dispatch(&vm, &vm.global);
		// ast_print(ast);
	}
	ast_free(&ast);
	return vm.status;
}