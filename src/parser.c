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
// See enum Token_types in token.h for the order
static const struct Operator op_priority[] = {
	{0, 0}, // T_UNKNOWN
	{10, 10}, {10, 10}, // '+', '-'
	{11, 11}, {11, 11}, // '*', '/'
	{3, 3},   {3, 3},   // '<', '>'
	{3, 3},   {3, 3},   // '==', '<='
	{3, 3},   {3, 3},   // '>=', '!='
	{11, 11},	// '%',
	{6, 6},	{4, 4}, {5, 5},	// '&', '|', '^'
	{7, 7}, {7, 7},	// '<<', '>>'
	{2, 2}, {1, 1},	// '&&', '||'
};

#define UNARY_PRIORITY 12

static int get_binop(struct Token token);
static int get_uop(struct Token token);
static int eof(struct Parser* p);
static int expect(struct Parser* p, enum Token_types expected_type);
static int expression_end(struct Parser* p);
static int statement(struct Parser* p);
static int statements(struct Parser* p);
static int simple_expr(struct Parser* p);
static int expr(struct Parser* p, int priority);

int get_binop(struct Token token) {
	if (token.type > T_UNKNOWN && token.type < T_NOBINOP)
		return token.type;
	return T_NOBINOP; // This is not a binary operator
}

int get_uop(struct Token token) {
	if ((token.type > T_NOBINOP && token.type < T_NOUNOP) || token.type == T_SUB)
		return token.type;
	return T_NOUNOP;  // This is not a unary operator
}

int eof(struct Parser* p) {
	struct Token token = get_token(p->lexer);
	return token.type == T_EOF;
}

int expect(struct Parser* p, enum Token_types expected_type) {
	struct Token token = get_token(p->lexer);
	return token.type == expected_type;
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

		case T_SEMICOLON:
		case T_NEWLINE:
			next_token(p->lexer);
			break;

		default:
			expr(p, 0);
			if (p->status != NO_ERR)
				return p->status;
			break;
	}

	return p->status;
}

int statements(struct Parser* p) {
	while (!eof(p))
		p->status = statement(p);
	return p->status;
}

int simple_expr(struct Parser* p) {
	struct Token token = get_token(p->lexer);

	switch (token.type) {
		case T_NUMBER:
			next_token(p->lexer);
			ast_add_node(p->ast, token);
			break;

		// The assignment case:
		// Output: { (expr) assign identifier }
		// No assignment case:
		// Output: { identifier }
		case T_IDENTIFIER: {
			struct Token identifier = token;
			next_token(p->lexer);
			if (expect(p, T_ASSIGN)) {
				struct Token assign_token = get_token(p->lexer);
				next_token(p->lexer); // Skip '='
				if (expect(p, T_NEWLINE) || expect(p, T_EOF)) {
					parseerror("Unexpected end of line in assignment\n");
					return p->status = PARSE_ERR;
				}
				statement(p); // Parse the right hand side statement
				ast_add_node(p->ast, assign_token);
			}
			ast_add_node(p->ast, identifier);
			if (expect(p, T_NEWLINE) || expect(p, T_SEMICOLON))
					next_token(p->lexer);
		}
			break;

		case T_NEWLINE:
			next_token(p->lexer);
			break;

		// let a = <value> ;
		// let a ;
		// The assignment case:
		// Output: { decl identifier (expr) assign identifier }
		// No assignment case:
		// Output: { decl identifier }
		case T_DECL: {
			ast_add_node(p->ast, token);	// 'let' token
			struct Token identifier = next_token(p->lexer); // Skip 'let'
			if (!expect(p, T_IDENTIFIER)) {
				parseerror("Expected identifier in declaration\n");
				return p->status = PARSE_ERR;
			}
			ast_add_node(p->ast, identifier); // Add identifier to ast
			next_token(p->lexer); // Skip identifier
			if (expect(p, T_ASSIGN)) {  // Variable assignment?
				struct Token assign_token = get_token(p->lexer);
				next_token(p->lexer); // Skip '='
				if (expect(p, T_NEWLINE) || expect(p, T_EOF)) {
					parseerror("Unexpected end of line\n");
					return p->status = PARSE_ERR;
				}
				statement(p); // Parse the right hand side statement
				ast_add_node(p->ast, assign_token);
				ast_add_node(p->ast, identifier);
			}
			if (expect(p, T_NEWLINE) || expect(p, T_SEMICOLON))
					next_token(p->lexer); // Skip '\n' or ';'
		}
			break;

		case T_OPENPAREN: {
			next_token(p->lexer); // Skip '('
			if (expression_end(p)) {
				parseerror("Expression can't be empty\n");
				return p->status = PARSE_ERR;
			}
			statement(p);
			if (!expression_end(p)) {
				parseerror("Missing ')' closing parenthesis in expression\n");
				return p->status = PARSE_ERR;
			}
			next_token(p->lexer); // Skip ')'
		}
			break;

		case T_RETURN:
			next_token(p->lexer);
			ast_add_node(p->ast, token);
			break;

		case T_EOF:
			return p->status;

		default:
			if (token.length > 0)
				parseerror("Unexpected symbol near '%.*s'\n", token.length, token.string);
			else
				parseerror("Unexpected symbol\n");
			next_token(p->lexer);
			return p->status = PARSE_ERR;
	}
	return p->status;
}

// Arithmetic operation on expressions
// expr op expr
int expr(struct Parser* p, int priority) {
	struct Token uop_token = get_token(p->lexer);
	int uop = get_uop(uop_token);
	if (uop != T_NOUNOP) {
		next_token(p->lexer); // Skip operator token
		expr(p, UNARY_PRIORITY);
		ast_add_node(p->ast, uop_token);
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
		ast_add_node(p->ast, token);
		op = next_op;
	}
	return op;
}

int parser_parse(char* input, Ast* ast) {
	struct Lexer lexer = {
		.index = input,
		.line = 1,
		.count = 0,
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