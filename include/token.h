// token.h

#ifndef _TOKEN_H
#define _TOKEN_H

enum Token_types {
	T_UNKNOWN = 0,

	T_ADD,
	T_SUB,
	T_MULT,
	T_DIV,
	T_LT,   // '<'
  T_GT,   // '>'
  T_EQ,   // '=='
  T_LEQ,  // '<='
  T_GEQ,  // '>='
  T_NEQ,  // '!='
	T_NOBINOP,
	
	T_NOT,	// '!'
	T_NOUNOP,

	T_ASSIGN,					// '='
	T_OPENPAREN,			// '('
	T_CLOSEDPAREN,		// ')'
	T_OPENBRACKET,		// '['
	T_CLOSEDBRACKET,	// ']'
	T_BLOCKBEGIN,			// '{'
	T_BLOCKEND,				// '}'

	T_IDENTIFIER,
	T_NUMBER,
	T_VOID,

	T_DECL_NUMBER,
	T_DECL_VOID,

	T_NEWLINE,
	T_EOF,

	T_COUNT
};

struct Token {
	int length;
	char* string;
	enum Token_types type;
};

void print_token(const struct Token token);

#endif