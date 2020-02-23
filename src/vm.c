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
	  struct Object left, right; \
	  left = vm->stack[vm->stack_top - 2]; \
	  right = vm->stack[vm->stack_top - 1]; \
	  if (OP_SAMETYPE(left, right, T_NUMBER)) { \
	    vm->stack[vm->stack_top - 2].value.number = left.value.number OP right.value.number; \
	    vm->stack_top--; \
	  } \
	  else vmerror("Invalid types in binary arithmetic operation\n"); \
	} \
} \

#define UNOP_ARITH(UOP) { \
	if (vm->stack_top > 0) { \
	  struct Object object = vm->stack[vm->stack_top - 1]; \
	  if (object.type == T_NUMBER) { \
	    vm->stack[vm->stack_top - 1].value.number = UOP(object.value.number); \
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
	if (vm->stack_top > 0)
		object_print(vm->stack[vm->stack_top - 1]);
	else
		printf("{}\n");
	return NO_ERR;
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
	for (int i = 0; i < vm->program_size; i++) {
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