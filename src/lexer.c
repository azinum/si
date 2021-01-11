// lexer.c

#include <assert.h>
#include <stdio.h>

#include "str.h"
#include "error.h"
#include "config.h"
#include "lexer.h"

#define MAX_BUFFER_LENGTH 2 << 9 // 1024
#define lexerror(fmt, ...) \
  error("%s:%i:%i: " COLOR_ERROR "lex-error: " COLOR_NONE fmt, lexer->filename, lexer->line, lexer->count, ##__VA_ARGS__)

static int is_alpha(char ch);
static int is_number(char ch);
static int endofline(struct Lexer* lexer);
static int match(struct Token token, const char* compare);
static void next(struct Lexer* lexer);
static struct Token read_number(struct Lexer* lexer);
static struct Token read_symbol(struct Lexer* lexer);

int is_alpha(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

int is_number(char ch) {
  return ch >= '0' && ch <= '9';
}

int endofline(struct Lexer* lexer) {
  return *lexer->index == '\n' || *lexer->index == '\r';
}

int match(struct Token token, const char* compare) {
  const char* index = compare;
  for (int i = 0; i < token.length; i++, index++) {
    if (token.string[i] != *index) {
      return 0;
    }
  }
  return (*index == 0);
}

void next(struct Lexer* lexer) {
  lexer->token.string = lexer->index++;
  lexer->token.length = 0;
  lexer->count++;
  lexer->token.count = lexer->count;
  lexer->token.line = lexer->line;
}

struct Token read_number(struct Lexer* lexer) {
  while (
        is_number(*lexer->index) ||
        *lexer->index == '.' ||
        *lexer->index == 'x' ||
        (*lexer->index >= 'a' && *lexer->index <= 'f') ||
        (*lexer->index >= 'A' && *lexer->index <= 'F')) {
    lexer->index++;
    lexer->count++;
  }
  lexer->token.length = lexer->index - lexer->token.string;
  double num = 0;
  if (safe_string_to_number(lexer->token.string, lexer->token.length, &num) != 0) {
    lexerror("Bad number\n");
    lexer->token.type = T_EOF;
    return lexer->token;
  }
  lexer->token.value.number = num;
  lexer->token.type = T_NUMBER;
  return lexer->token;
}

struct Token read_symbol(struct Lexer* lexer) {
  while (
      is_alpha(*lexer->index) ||
      is_number(*lexer->index) ||
      *lexer->index == '_'
  ) {
    lexer->index++;
    lexer->count++;
  }
  lexer->token.length = lexer->index - lexer->token.string;
  if (match(lexer->token, TOKEN_DECL))
    lexer->token.type = T_DECL;
  else if (match(lexer->token, TOKEN_RETURN))
    lexer->token.type = T_RETURN;
  else if (match(lexer->token, TOKEN_IF))
    lexer->token.type = T_IF;
  else if (match(lexer->token, TOKEN_WHILE))
    lexer->token.type = T_WHILE;
  else if (match(lexer->token, TOKEN_BREAK))
    lexer->token.type = T_BREAK;
  else if (match(lexer->token, TOKEN_FUNC_DEF))
    lexer->token.type = T_FUNC_DEF;
  else if (match(lexer->token, TOKEN_IMPORT))
    lexer->token.type = T_IMPORT;
  else if (match(lexer->token, TOKEN_LOAD))
    lexer->token.type = T_LOAD;
  else if (match(lexer->token, TOKEN_NIL))
    lexer->token.type = T_NIL;
  else
    lexer->token.type = T_IDENTIFIER;
  return lexer->token;
}

struct Token next_token(struct Lexer* lexer) {
  assert(lexer != NULL);
  for (;;) {
begin_loop:
    next(lexer);
    char ch = *lexer->token.string;
    switch (ch) {
      case '\n':
      case '\r':
        lexer->line++;
        lexer->count = 0;
        break;

      // Skip whitespaces
      case ' ':
      case '\t':
      case '\v':
      case '\f':
        break;

      // Shebang!
      case '#': {
        if (*lexer->index == '!') {
          next(lexer);
          while (!endofline(lexer) && *lexer->index != '\0')
            next(lexer);
          break;
        }
        lexerror("Invalid token\n");
        lexer->token.type = T_EOF;
        return lexer->token;
      }

      case '+':
        lexer->token.type = T_ADD;
        return lexer->token;

      case '-':
        lexer->token.type = T_SUB;
        return lexer->token;

      case '*':
        lexer->token.type = T_MULT;
        return lexer->token;

      case '"':
      case '\'': {
        char to_match = ch;
        while (1) {
          if ((*lexer->index) == '\0') {
            lexerror("Unfinished string; missing terminating character (%c)\n", to_match);
            break;
          }
          if ((*lexer->index) == to_match) {
            break;
          }
          lexer->index++;
          lexer->count++;
        }
        lexer->token.string++;
        lexer->token.length = lexer->index - lexer->token.string;
        lexer->token.type = T_STRING;
        lexer->index++;
        return lexer->token;
      }

      case '/': {
        if (*lexer->index == '/') { // Single line comment
          next(lexer);
          while (!endofline(lexer) && *lexer->index != '\0')
            next(lexer);
          break;
        }
        else if (*lexer->index == '*') {  // Multi-line comment
          next(lexer);
          while (*lexer->index != '\0') {
            if (*lexer->index == '*') {
              next(lexer);
              if (*lexer->index == '/') {
                next(lexer);
                goto begin_loop;
              }
            }
            else if (endofline(lexer)) {
              lexer->line++;
              lexer->count = 1;
              next(lexer);
            }
            else
              next(lexer);
          }
          lexerror("Unfinished multi-line comment\n");
          lexer->token.type = T_EOF;  // Just to be safe
          return lexer->token;
        }
        lexer->token.type = T_DIV;
        return lexer->token;
      }

      case '<': {
        if (*lexer->index == '=') {
          lexer->token.type = T_LEQ;
          lexer->index++;
          return lexer->token;
        }
        if (*lexer->index == '<') {
          lexer->token.type = T_LEFTSHIFT;
          lexer->index++;
          return lexer->token;
        }
        lexer->token.type = T_LT;
        return lexer->token;
      }

      case '>': {
        if (*lexer->index == '=') {
          lexer->token.type = T_GEQ;
          lexer->index++;
          return lexer->token;
        }
        if (*lexer->index == '>') {
          lexer->token.type = T_RIGHTSHIFT;
          lexer->index++;
          return lexer->token;
        }
        lexer->token.type = T_GT;
        return lexer->token;
      }

      case '%':
        lexer->token.type = T_MOD;
        return lexer->token;

      case '&': {
        if (*lexer->index == '&') {
          lexer->token.type = T_AND;
          lexer->index++;
          return lexer->token;
        }
        lexer->token.type = T_BAND;
        return lexer->token;
      }

      case '|': {
        if (*lexer->index == '|') {
          lexer->token.type = T_OR;
          lexer->index++;
          return lexer->token;
        }
        lexer->token.type = T_BOR;
        return lexer->token;
      }

      case '^':
        lexer->token.type = T_BXOR;
        return lexer->token;

      case '!': {
        if (*lexer->index == '=') {
          lexer->token.type = T_NEQ;
          lexer->index++;
          return lexer->token;
        }
        lexer->token.type = T_NOT;
        return lexer->token;
      }

      case '=': {
        if (*lexer->index == '=') {
          lexer->token.type = T_EQ;
          lexer->index++;
          return lexer->token;
        }
        lexer->token.type = T_ASSIGN;
        return lexer->token;
      }

      case '(':
        lexer->token.type = T_OPENPAREN;
        return lexer->token;

      case ')':
        lexer->token.type = T_CLOSEDPAREN;
        return lexer->token;

      case '[':
        lexer->token.type = T_OPENBRACKET;
        return lexer->token;

      case ']':
        lexer->token.type = T_CLOSEDBRACKET;
        return lexer->token;

      case '{':
        lexer->token.type = T_BLOCKBEGIN;
        return lexer->token;

      case '}':
        lexer->token.type = T_BLOCKEND;
        return lexer->token;

      case ';':
        lexer->token.type = T_SEMICOLON;
        return lexer->token;

      case ':':
        lexer->token.type = T_COLON;
        return lexer->token;

      case ',':
        lexer->token.type = T_COMMA;
        return lexer->token;

      case '\0':
        lexer->token.type = T_EOF;
        return lexer->token;

      default: {
        if (is_number(ch)) {
          return read_number(lexer);
        }
        else if (is_alpha(ch) || ch == '_') {
          return read_symbol(lexer);
        }
        else {
          lexerror("Unrecognized character\n");
          lexer->token.type = T_EOF;
          return lexer->token;
        }
        break;
      }
    }
  }
  return lexer->token;
}

// Get current token
struct Token get_token(struct Lexer* lexer) {
  assert(lexer != NULL);
  return lexer->token;
}
