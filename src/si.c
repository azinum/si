// si.c
// simple interpreter
// author: lucas (azinum)

#include "si.h"
#include "error.h"
#include "token.h"
#include "ast.h"
#include "parser.h"
#include "file.h"

int si_exec(int argc, char** argv) {
	{
		char* input = read_file("scripts/test.lang");
		if (input) {
			parser_parse(input);
			free(input);
		}
	}
	return NO_ERR;
}