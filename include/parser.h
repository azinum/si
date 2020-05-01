// parser.h

#ifndef _PARSER_H
#define _PARSER_H

#define PATH_LENGTH_MAX 512

int parser_parse(char* input, struct Buffer* buff, const char* filename, Ast* ast);

#endif
