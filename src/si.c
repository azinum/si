// si.c
// simple interpreter
// author: lucas (azinum)

#include "si.h"
#include "error.h"
#include "token.h"
#include "ast.h"
#include "parser.h"
#include "file.h"
#include "config.h"

int si_exec(int argc, char** argv) {
	{
		Ast ast = create_ast();
		char* input = read_file("scripts/test.lang");
		if (input) {
			parser_parse(input, &ast);
			print_ast(ast);
			free_ast(&ast);
			ast = NULL;
			free(input);
		}
	}

/*
	char input[INPUT_MAX] = {0};
	int status = NO_ERR;
	int is_running = 1;
  while (is_running) {
    printf("> ");
    if (fgets(input, INPUT_MAX, stdin) != NULL) {
      status = parser_parse(input);
      if (status != NO_ERR) return status;
    }
    else is_running = 0;
  }
*/

	return NO_ERR;
}