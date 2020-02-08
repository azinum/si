// compile.c
// node tree -> instruction sequence

#include <assert.h>
#include <stdlib.h>

#include "error.h"
#include "ast.h"
#include "vm.h"
#include "object.h"
#include "compile.h"

static int seq_init(struct Ins_seq* seq);

int seq_init(struct Ins_seq* seq) {
	assert(seq != NULL);
	seq->sequence = NULL;
	seq->count = 0;
	return NO_ERR;
}

int compile_from_tree(struct Ins_seq* seq, Ast* ast, int level) {
	assert(ast != NULL);
	assert(seq != NULL);
	seq_init(seq);
	if (ast_is_empty(*ast)) return NO_ERR;

	return NO_ERR;
}
