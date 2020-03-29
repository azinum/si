// parser.h

#ifndef _PARSER_H
#define _PARSER_H

typedef struct Node* Ast;

int parser_parse(char* input, char* filename, Ast* ast);

#endif