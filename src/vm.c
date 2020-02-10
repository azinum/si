// vm.c

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "ast.h"
#include "hash.h"
#include "parser.h"
#include "compile.h"
#include "vm.h"

struct VM_state {
	int status;
	struct Ins_seq seq;
};

static int vm_init(struct VM_state* vm);

int vm_init(struct VM_state* vm) {
	assert(vm != NULL);
	vm->status = NO_ERR;
	return vm->status;
}

int vm_exec(char* input) {
	assert(input != NULL);
	struct VM_state vm = {};
	int status = NO_ERR;
	vm_init(&vm);

	unsigned int size = 347;
	Htable table = ht_create(size);
	ht_insert_element(&table, "a", 12);
	ht_insert_element(&table, "a", 14);
	ht_insert_element(&table, "a1", 20);
	ht_remove_element(&table, "a1");
	ht_insert_element(&table, "a1", 30);
	ht_remove_element(&table, "a");
	printf("%i\n", ht_num_elements(&table));
	ht_free(&table);

	// Ast ast = create_ast();
	// status = parser_parse(input, &ast);
	// if (status == NO_ERR) {
	// 	status = compile_from_tree(&vm.seq, &ast, 0);
	// }
	// free_ast(&ast);
	return vm.status;
}