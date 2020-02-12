// token.c

#include <stdio.h>

#include "token.h"

void print_token(const struct Token token) {
	if (token.length > 0)
		printf("%.*s\n", token.length, token.string);
}