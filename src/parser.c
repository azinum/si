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
	(error("%s:%i:%i: " COLOR_ERROR "parse-error: " COLOR_NONE fmt, p->lexer->filename, p->lexer->line, p->lexer->count, ##__VA_ARGS__))

struct Parser {
	struct Lexer* lexer;
	Ast* ast;
	int status;
	int loop;	// Are we in a loop block?
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
static int block_end(struct Parser* p);
static int expect(struct Parser* p, enum Token_types expected_type);
static void check(struct Parser* p, enum Token_types type);
static void skip(struct Parser* p);
static int expression_end(struct Parser* p);
static int declare_variable(struct Parser* p);
static int ifstatement(struct Parser* p);
static int whileloop(struct Parser* p);
static int block(struct Parser* p);
static int breakstat(struct Parser* p);
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

int block_end(struct Parser* p) {
	struct Token token = get_token(p->lexer);
	switch (token.type) {
		case T_EOF:
		case T_BLOCKEND:
			return 1;
		default:
			return 0;
	}
	return 0;
}

int expect(struct Parser* p, enum Token_types expected_type) {
	struct Token token = get_token(p->lexer);
	return token.type == expected_type;
}

void check(struct Parser* p, enum Token_types type) {
	struct Token token = get_token(p->lexer);
	if (token.type != type) {
		if (token.length > 0)
			parseerror("Unexpected symbol '%.*s'\n", token.length, token.string);
		else
			parseerror("Unexpected symbol\n");
		p->status = PARSE_ERR;
	}
}

// Skip newlines and go to next token
void skip(struct Parser* p) {
	struct Token token = get_token(p->lexer);
	while (token.type == T_SEMICOLON)
		token = next_token(p->lexer);
}

int expression_end(struct Parser* p) {
	struct Token token = get_token(p->lexer);
	return token.type == T_CLOSEDPAREN;
}

// let a = <value> ;
// Output:
// \--> decl
// \--> ident
//   \--> (expr)
int declare_variable(struct Parser* p) {
	struct Token decl_token = get_token(p->lexer);
	next_token(p->lexer); // Skip 'let'
	ast_add_node(p->ast, decl_token);
	struct Token identifier = get_token(p->lexer);
	ast_add_node(p->ast, identifier);
	next_token(p->lexer);	// Skip 'identifier'
	if (!expect(p, T_ASSIGN)) {
		parseerror("Expected '='\n");
		return p->status = PARSE_ERR;
	}
	next_token(p->lexer);	// Skip '='
	Ast* orig_branch = p->ast;
	Ast expr_branch = ast_get_last(orig_branch);
	p->ast = &expr_branch;
	expr(p, 0);
	assert(ast_child_count(&expr_branch) > 0);
	p->ast = orig_branch;
	return NO_ERR;
}

// if COND {}
// Ast output:
// \--> if
//   \--> COND
// \--> T_BLOCK
//   \--> { BLOCK }
int ifstatement(struct Parser* p) {
	struct Token if_node = get_token(p->lexer);
	next_token(p->lexer);	// Skip 'if'
	Ast* orig_branch = p->ast;
	ast_add_node(orig_branch, if_node);	// Add if node
	Ast cond_branch = ast_get_last(orig_branch);
	p->ast = &cond_branch;
	expr(p, 0);	// Read condition
	if (!expect(p, T_BLOCKBEGIN)) {
		parseerror("Expected '{' block begin after condition\n");
		return p->status = PARSE_ERR;
	}
	struct Token block_begin = { .type = T_BLOCK };
	ast_add_node(orig_branch, block_begin);
	next_token(p->lexer);	// Skip '{'
	Ast block_branch = ast_get_last(orig_branch);
	p->ast = &block_branch;
	block(p);
	p->ast = orig_branch;
	return NO_ERR;
}

// Ast output:
// \--> while
//   \--> COND
// \--> T_BLOCK
//   \--> { BLOCK }
int whileloop(struct Parser* p) {
	p->loop++;	// We are now in a while loop block (increment for nested loops)
	struct Token while_node = get_token(p->lexer);
	next_token(p->lexer);	// Skip 'while'
	Ast* orig_branch = p->ast;
	ast_add_node(orig_branch, while_node);	// Add while node
	Ast cond_branch = ast_get_last(orig_branch);
	p->ast = &cond_branch;
	expr(p, 0);	// Read condition
	if (!expect(p, T_BLOCKBEGIN)) {
		parseerror("Expected '{' block begin\n");
		return p->status = PARSE_ERR;
	}
	struct Token block_begin = { .type = T_BLOCK };
	ast_add_node(orig_branch, block_begin);
	next_token(p->lexer);	// Skip '{'
	Ast block_branch = ast_get_last(orig_branch);
	p->ast = &block_branch;
	block(p);
	p->ast = orig_branch;
	p->loop--;	// Exit this loop block
	return NO_ERR;
}

int block(struct Parser* p) {
	statements(p);
	if (!expect(p, T_BLOCKEND)) {
		parseerror("Expected '}' block end\n");
		return p->status = PARSE_ERR;
	}
	next_token(p->lexer);
	return NO_ERR;
}

int breakstat(struct Parser* p) {
	struct Token token = get_token(p->lexer);
	next_token(p->lexer);
	if (!p->loop) {
		parseerror("No loop to break\n");
		return p->status = PARSE_ERR;
	}
	ast_add_node(p->ast, token);
	if (!expect(p, T_SEMICOLON)) {
		parseerror("Expected ';'\n");
		return p->status = PARSE_ERR;
	}
	return NO_ERR;
}

int statement(struct Parser* p) {
	struct Token token = get_token(p->lexer);
	switch (token.type) {
		case T_EOF:
			return NO_ERR;

		case T_SEMICOLON:
			next_token(p->lexer);
			break;

		case T_DECL:
			declare_variable(p);
			break;

		case T_WHILE:
			whileloop(p);
			break;

		case T_BREAK:
			breakstat(p);
			break;

		case T_IF:
			ifstatement(p);
			break;

		// return ;
		// return (expr) ;
		// Output: { (expr) return }
		case T_RETURN: {
			struct Token return_node = token;
			next_token(p->lexer);	// Skip 'return'
			expr(p, 0);
			ast_add_node(p->ast, return_node);
			break;
		}

		default:
			expr(p, 0);
			break;
	}
	if (p->status != NO_ERR)
		return p->status;
	return p->status;
}

int statements(struct Parser* p) {
	while (!block_end(p))
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

		case T_IDENTIFIER:
			next_token(p->lexer);
			if (expect(p, T_ASSIGN)) {  // Variable assignment?
				struct Token assign_token = get_token(p->lexer);
				next_token(p->lexer); // Skip '='
				if (expect(p, T_EOF)) {
					parseerror("Unexpected EOF\n");
					return p->status = PARSE_ERR;
				}
				expr(p, 0); // Parse the right hand side expression
				ast_add_node(p->ast, assign_token);
			}
			ast_add_node(p->ast, token); // Add identifier to ast
			break;

		case T_OPENPAREN: {
			next_token(p->lexer); // Skip '('
			if (expression_end(p)) {
				parseerror("Expression can't be empty\n");
				return p->status = PARSE_ERR;
			}
			expr(p, 0);
			if (!expression_end(p)) {
				parseerror("Missing ')' closing parenthesis in expression\n");
				return p->status = PARSE_ERR;
			}
			next_token(p->lexer); // Skip ')'
			break;
		}

		default:
			if (token.length > 0)
				parseerror("Unexpected symbol '%.*s'\n", token.length, token.string);
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

int parser_parse(char* input, const char* filename, Ast* ast) {
	struct Lexer lexer = {
		.index = input,
		.line = 1,
		.count = 0,
		.token = (struct Token) {0},
		.filename = filename,
	};
	struct Parser parser = {
		.lexer = &lexer,
		.ast = ast,
		.status = NO_ERR,
		.loop = 0
	};
	next_token(parser.lexer);
	statements(&parser);
	check(&parser, T_EOF);
	return parser.status;
}