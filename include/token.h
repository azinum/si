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
  T_MOD,	// '%'
  T_BAND,		// Bitwise '&'
  T_BOR,		// '|'
  T_BXOR,		// '^'
  T_LEFTSHIFT,	// '<<'
  T_RIGHTSHIFT,	// '>>'
  T_AND,	// '&&'
  T_OR,		// '||'
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
	T_SEMICOLON,			// ';'

	T_IDENTIFIER,
	T_NUMBER,

	T_DECL,	// 'let'
	T_RETURN,
	T_IF,
	T_WHILE,

	T_BLOCK,

	T_NEWLINE,
	T_EOF,

	T_COUNT
};

#define TOKEN_DECL 		"let"
#define TOKEN_RETURN 	"return"
#define TOKEN_IF			"if"
#define TOKEN_WHILE		"while"

struct Token {
	int length;
	char* string;
	enum Token_types type;
};

void print_token(const struct Token token);

#endif