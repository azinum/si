// parser.c

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "token.h"
#include "ast.h"
#include "lexer.h"
#include "error.h"
#include "parser.h"

#define parseerror(fmt, ...) \
	error("%i:%i: " COLOR_ERROR "parse-error: " COLOR_NONE fmt, p->lexer->line, p->lexer->count, ##__VA_ARGS__)

struct Parser {
	struct Lexer* lexer;
	Ast* ast;
	int status;
};

struct Operator {
	int left, right;
};

// Warning: Order is reserved. DO NOT CHANGE.
static const struct Operator op_priority[] = {
	{0, 0},	// T_UNKNOWN
	{10, 10}, {10, 10},	// T_PLUS, T_MINUS
	{11, 11}, {11, 11},	// T_MULT, T_DIV
	{3, 3},		{3, 3},		// T_LT, T_GT
	{3, 3},		{3, 3},		// T_EQ, T_LEQ
	{3, 3},		{3, 3},		// T_GEQ, T_NEQ
};

#define UNARY_PRIORITY 12

static int get_binop(struct Token token);
static int get_uop(struct Token token);
static int eof(struct Parser* p);
static int expression_end(struct Parser* p);
static int statement(struct Parser* p);
static int statements(struct Parser* p);
static int simple_expr(struct Parser* p);
static int expr(struct Parser* p, int priority);

int get_binop(struct Token token) {
	if (token.type > T_UNKNOWN && token.type < T_NOBINOP)
		return token.type;
	return T_NOBINOP;	// This is not a binary operator
}

int get_uop(struct Token token) {
	if ((token.type > T_NOBINOP && token.type < T_NOUNOP) || token.type == T_MINUS)
		return token.type;
	return T_NOUNOP;	// This is not a unary operator
}

int eof(struct Parser* p) {
	struct Token token = get_token(p->lexer);
	return token.type == T_EOF;
}

int expression_end(struct Parser* p) {
	struct Token token = get_token(p->lexer);
	return token.type == T_CLOSEDPAREN;
}

int statement(struct Parser* p) {
	struct Token token = get_token(p->lexer);

	switch (token.type) {
		case T_EOF:
			return NO_ERR;

		case T_NEWLINE:
			next_token(p->lexer);
			break;

		default:
			expr(p, 0);
			break;
	}

	return NO_ERR;
}

int statements(struct Parser* p) {
	while (!eof(p)) p->status = statement(p);
	return p->status;
}

int simple_expr(struct Parser* p) {
	struct Token token = get_token(p->lexer);

	switch (token.type) {
		case T_NUMBER:
			next_token(p->lexer);
			add_ast_node(p->ast, token);
			break;

		case T_IDENTIFIER:
			next_token(p->lexer);
			add_ast_node(p->ast, token);
			break;

		case T_OPENPAREN: {
			next_token(p->lexer);	// Skip '('
			if (expression_end(p)) {
				parseerror("Expression can't be empty\n");
				return (p->status = PARSE_ERR);
			}
			statement(p);
			if (!expression_end(p)) {
				parseerror("Missing ')' closing parenthesis in expression\n");
				return (p->status = PARSE_ERR);
			}
			next_token(p->lexer);	// Skip ')'
		}
			break;

		default:
			next_token(p->lexer);
			break;
	}
	return NO_ERR;
}

// Arithmetic operation on expressions
// expr op expr
int expr(struct Parser* p, int priority) {
	struct Token uop_token = get_token(p->lexer);
	int uop = get_uop(uop_token);
	if (uop != T_NOUNOP) {
		next_token(p->lexer);	// Skip operator token
		expr(p, UNARY_PRIORITY);
		add_ast_node(p->ast, uop_token);
	}
	else {
		p->status = simple_expr(p);
	}

	struct Token token = get_token(p->lexer);
	int op = get_binop(token);

	while (op != T_NOBINOP && op_priority[op].left > priority) {
		int next_op;
		token = get_token(p->lexer);
		next_token(p->lexer);
		next_op = expr(p, op_priority[op].right);
		add_ast_node(p->ast, token);
		op = next_op;
	}
	return op;
}

int parser_parse(char* input, Ast* ast) {
	struct Lexer lexer = {
		.index = input,
		.line = 1,
		.count = 1,
		.token = (struct Token) {0}
	};
	struct Parser parser = {
		.lexer = &lexer,
		.ast = ast,
		.status = NO_ERR
	};
	next_token(parser.lexer);
	statements(&parser);
	return parser.status;
}