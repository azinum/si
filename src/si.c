// si.c
// simple interpreter
// author: lucas (azinum)

#include "si.h"
#include "error.h"
#include "token.h"
#include "ast.h"
#include "parser.h"

#define E_TOKEN (struct Token) {}

int si_exec(int argc, char** argv) {
	char input[] = "5 + 2 + 12 * 2 + (2 + 3)";
	parser_parse(input);
	return NO_ERR;
}