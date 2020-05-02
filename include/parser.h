// parser.h

#ifndef _PARSER_H
#define _PARSER_H

#define PATH_LENGTH_MAX 512

int parser_parse(char* input, struct Str_arr* str_arr, const char* filename, Ast* ast);

#endif
