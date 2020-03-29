// lexer.h

#ifndef _LEXER_H
#define _LEXER_H

#include "token.h"

struct Lexer {
	char* index;
	int line, count;
	struct Token token;
	char* filename;
};

struct Token next_token(struct Lexer* lexer);

struct Token get_token(struct Lexer* lexer);

#endif