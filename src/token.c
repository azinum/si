// token.c

#include <stdio.h>

#include "token.h"

const char* token_desc[T_COUNT] = {
  "unknown",

  "+",
  "-",
  "*",
  "/",
  "<",
  ">",
  "==",
  "<=",
  ">=",
  "!=",
  "%",
  "&",
  "|",
  "^",
  "<<",
  ">>",
  "&&",
  "||",
  "", // T_NOBINOP

  "!",
  "", // T_NOUNOP

  "=",
  "(",
  ")",
  "[",
  "]",
  "{",
  "}",
  ";",
  ",",

  "identifier",
  "number",
  "string",
  "function",
  "c function",
  "list",
  TOKEN_NIL,

  TOKEN_DECL,
  TOKEN_RETURN,
  TOKEN_IF,
  TOKEN_WHILE,
  TOKEN_BREAK,
  TOKEN_FUNC_DEF,
  TOKEN_IMPORT,
  TOKEN_LOAD,

  "call",
  "block",
  "EOF",
};

void print_token(struct Token token) {
  switch (token.type) {
    case T_NUMBER:
      printf("%g", token.value.number);
      break;

    default:
      printf("%s", token_desc[token.type]);
      break;
  }
}
