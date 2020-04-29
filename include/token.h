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
  T_COMMA,          // ','

	T_IDENTIFIER,	// 31
	T_NUMBER,			// 32
	T_FUNCTION,		// 33
	T_NIL,

	T_DECL,	// 'let'
	T_RETURN,
	T_IF,
	T_WHILE,
	T_BREAK,
	T_FUNC_DEF,

  T_CALL,
	T_BLOCK,
	T_EOF,

	T_COUNT
};

#define TOKEN_DECL "let"
#define TOKEN_RETURN "return"
#define TOKEN_IF "if"
#define TOKEN_WHILE "while"
#define TOKEN_BREAK "break"
#define TOKEN_FUNC_DEF "fn"

struct Token {
	int type;
	char* string;
	int length;
	union {
		double number;
		int integer;
	} value;
};

#endif