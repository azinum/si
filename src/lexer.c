// lexer.c

#include <assert.h>
#include <stdio.h>

#include "error.h"
#include "config.h"
#include "lexer.h"

#define MAX_BUFFER_LENGTH 2 << 9 // 1024
#define lexerror(fmt, ...) \
	error("%i:%i: " COLOR_ERROR "lex-error: " COLOR_NONE fmt, lexer->line, lexer->count, ##__VA_ARGS__)

static int is_alpha(char ch);
static int is_number(char ch);
static int is_whitespace(char ch);
static int is_endofline(char ch);
static int remove_whitespaces(struct Lexer* lexer);
static int remove_newlines(struct Lexer* lexer);
static int token_equals(struct Token token, const char* match);

int is_alpha(char ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

int is_number(char ch) {
	return ch >= '0' && ch <= '9';
}

int is_whitespace(char ch) {
	return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\f';
}

int is_endofline(char ch) {
	return ch == '\n' || ch == '\r';
}

int remove_whitespaces(struct Lexer* lexer) {
	while (is_whitespace(lexer->index[0])) {
		lexer->index++;
		lexer->count++;
	}
	return NO_ERR;
}

int remove_newlines(struct Lexer* lexer) {
	while (is_endofline(lexer->index[0])) {
		lexer->index++;
		lexer->count++;
		lexer->line++;
	}
	return NO_ERR;
}

int token_equals(struct Token token, const char* match) {
  const char* index = match;
  for (int i = 0; i < token.length; i++, index++) {
    if (token.string[i] != *index) {
      return 0;
    }
  }
  return (*index == 0);
}

// Jump to next token
struct Token next_token(struct Lexer* lexer) {
	(void)is_endofline;
	assert(lexer != NULL);
	(void)remove_newlines;
	remove_whitespaces(lexer);
	struct Token token = {1, lexer->index, T_UNKNOWN};

	char ch = lexer->index[0];
	lexer->index++;
	switch (ch) {
		case '+':
			token.type = T_ADD;
			break;

		case '-':
			token.type = T_SUB;
			break;

		case '*':
			token.type = T_MULT;
			break;

		case '/':
			token.type = T_DIV;
			break;

		case '<': {
			if (lexer->index[0] == '=') {
        lexer->index++;
        token.type = T_LEQ;
        token.length = 2;
        break;
      }
			token.type = T_LT;
		}
			break;

		case '>': {
			if (lexer->index[0] == '=') {
        lexer->index++;
        token.type = T_GEQ;
        token.length = 2;
        break;
      }
			token.type = T_GT;
		}
			break;

		case '=': {
			if (lexer->index[0] == '=') {	// Is next character also a '='?
        lexer->index++;
        token.type = T_EQ;
        token.length = 2;
        break;
      }
      token.type = T_ASSIGN;
    }
			break;

		case '!': {
			if (lexer->index[0] == '=') {
        lexer->index++;
        token.type = T_NEQ;
        token.length = 2;
        break;
      }
			token.type = T_NOT;
		}	
			break;

		case '(':
			token.type = T_OPENPAREN;
			break;

		case ')':
			token.type = T_CLOSEDPAREN;
			break;

		case '[':
			token.type = T_OPENBRACKET;
			break;

		case ']':
			token.type = T_CLOSEDBRACKET;
			break;

		case '{':
			token.type = T_BLOCKBEGIN;
			break;

		case '}':
			token.type = T_BLOCKEND;
			break;

		case ';':
			token.type = T_SEMICOLON;
			break;

		case '\n':
		case '\r':
			token.type = T_NEWLINE;
			break;

		case '\0':
			token.type = T_EOF;
			break;

		default: {
			if (is_alpha(ch) || ch == '_') {
				while (
					(is_alpha(lexer->index[0]) ||
					is_number(lexer->index[0]) ||
					lexer->index[0] == '_')) {
						lexer->index++;
				}
				token.length = lexer->index - token.string;

				if (token_equals(token, DECL_NUMBER))
					token.type = T_DECL_NUMBER;
				else if (token_equals(token, DECL_VOID))
					token.type = T_DECL_VOID;
				else
					token.type = T_IDENTIFIER;
			}
			else if (is_number(ch) || ch == '.') {
				int dot_count = 0;
				while (is_number(lexer->index[0]) || lexer->index[0] == '.') {
					if (lexer->index[0] == '.') dot_count++;
					if (dot_count > 1) {
						lexerror("Invalid number\n");
						break;
					}
					lexer->index++;
				}
				token.type = T_NUMBER;
				token.length = lexer->index - token.string;
			}
			else {
				token.length = 0;
        token.type = T_UNKNOWN;
      }
     }
				break;
	}
	lexer->token = token;
	lexer->count += token.length;
	return token;
}

// Get current token
struct Token get_token(struct Lexer* lexer) {
	assert(lexer != NULL);
	return lexer->token;
}