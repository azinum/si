// parser.c

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "file.h"
#include "strarr.h"
#include "token.h"
#include "ast.h"
#include "lexer.h"
#include "error.h"
#include "parser.h"

#define parseerror(fmt, ...) \
  (error("%s:%i:%i: " COLOR_ERROR "parse-error: " COLOR_NONE fmt, p->lexer->filename, p->lexer->line, p->lexer->count, ##__VA_ARGS__))

struct Parser {
  struct Lexer* lexer;
  Ast* ast;
  int status;
  int loop; // Are we in a loop block?
  struct Str_arr* str_arr;
};

struct Operator {
  int left, right;
};

// Warning: Order is reserved. DO NOT CHANGE.
// See enum Token_types in token.h for the order
static const struct Operator op_priority[] = {
  {0, 0}, // T_UNKNOWN
  {10, 10}, {10, 10}, // '+', '-'
  {11, 11}, {11, 11}, // '*', '/'
  {3, 3},   {3, 3},   // '<', '>'
  {3, 3},   {3, 3},   // '==', '<='
  {3, 3},   {3, 3},   // '>=', '!='
  {11, 11}, // '%',
  {6, 6}, {4, 4}, {5, 5}, // '&', '|', '^'
  {7, 7}, {7, 7}, // '<<', '>>'
  {2, 2}, {1, 1}, // '&&', '||'
};

#define UNARY_PRIORITY 12

static int add_current_token(struct Parser* p);
static int get_binop(struct Token token);
static int get_uop(struct Token token);
static int block_end(struct Parser* p);
static int expect(struct Parser* p, enum Token_types expected_type);
static void check(struct Parser* p, enum Token_types type);
static int expression_end(struct Parser* p);
static int declare_variable(struct Parser* p);
static int ifstatement(struct Parser* p);
static int whileloop(struct Parser* p);
static int returnstat(struct Parser* p);
static int importstat(struct Parser* p);
static int loadstat(struct Parser* p);
static int params(struct Parser* p);
static int arglist(struct Parser* p, int* num_args);
static int funcstat(struct Parser* p);
static int block(struct Parser* p);
static int breakstat(struct Parser* p);
static int statement(struct Parser* p);
static int statements(struct Parser* p);
static int postfix_expr(struct Parser* p);
static int simple_expr(struct Parser* p);
static int expr(struct Parser* p, int priority);

// Add current token to ast and move on the the next one
int add_current_token(struct Parser* p) {
  struct Token token = get_token(p->lexer);
  next_token(p->lexer);
  ast_add_node(p->ast, token);
  return NO_ERR;
}

int get_binop(struct Token token) {
  if (token.type > T_UNKNOWN && token.type < T_NOBINOP)
    return token.type;
  return T_NOBINOP; // This is not a binary operator
}

int get_uop(struct Token token) {
  if ((token.type > T_NOBINOP && token.type < T_NOUNOP) || token.type == T_SUB)
    return token.type;
  return T_NOUNOP;  // This is not a unary operator
}

int block_end(struct Parser* p) {
  struct Token token = get_token(p->lexer);
  switch (token.type) {
    case T_EOF:
    case T_BLOCKEND:
      return 1;
    default:
      return 0;
  }
  return 0;
}

int expect(struct Parser* p, enum Token_types expected_type) {
  struct Token token = get_token(p->lexer);
  return token.type == expected_type;
}

void check(struct Parser* p, enum Token_types type) {
  struct Token token = get_token(p->lexer);
  if (token.type != type) {
    parseerror("Unexpected symbol\n");
    p->status = PARSE_ERR;
  }
}

int expression_end(struct Parser* p) {
  struct Token token = get_token(p->lexer);
  return token.type == T_CLOSEDPAREN;
}

// let a = <value> ;
// Output:
// \--> decl
// \--> ident
//   \--> (expr)
int declare_variable(struct Parser* p) {
  struct Token decl_token = get_token(p->lexer);
  next_token(p->lexer); // Skip 'let'
  ast_add_node(p->ast, decl_token);
  struct Token identifier = get_token(p->lexer);
  ast_add_node(p->ast, identifier);
  next_token(p->lexer); // Skip 'identifier'
  if (!expect(p, T_ASSIGN)) {
    parseerror("Expected '='\n");
    return p->status = PARSE_ERR;
  }
  next_token(p->lexer); // Skip '='
  Ast* orig_branch = p->ast;
  Ast expr_branch = ast_get_last(orig_branch);
  p->ast = &expr_branch;
  expr(p, 0);
  p->ast = orig_branch;
  return NO_ERR;
}

// if COND {}
// Ast output:
// \--> if
//   \--> COND
// \--> T_BLOCK
//   \--> { BLOCK }
int ifstatement(struct Parser* p) {
  struct Token if_node = get_token(p->lexer);
  next_token(p->lexer); // Skip 'if'
  Ast* orig_branch = p->ast;
  ast_add_node(orig_branch, if_node); // Add if node
  Ast cond_branch = ast_get_last(orig_branch);
  p->ast = &cond_branch;
  expr(p, 0); // Read condition
  if (!expect(p, T_BLOCKBEGIN)) {
    parseerror("Expected '{' block begin after condition\n");
    return p->status = PARSE_ERR;
  }
  struct Token block_begin = { .type = T_BLOCK };
  ast_add_node(orig_branch, block_begin);
  next_token(p->lexer); // Skip '{'
  Ast block_branch = ast_get_last(orig_branch);
  p->ast = &block_branch;
  block(p);
  p->ast = orig_branch;
  return NO_ERR;
}

// Ast output:
// \--> while
//   \--> COND
// \--> T_BLOCK
//   \--> { BLOCK }
int whileloop(struct Parser* p) {
  p->loop++;  // We are now in a while loop block (increment for nested loops)
  struct Token while_node = get_token(p->lexer);
  next_token(p->lexer); // Skip 'while'
  Ast* orig_branch = p->ast;
  ast_add_node(orig_branch, while_node);  // Add while node
  Ast cond_branch = ast_get_last(orig_branch);
  p->ast = &cond_branch;
  expr(p, 0); // Read condition
  if (!expect(p, T_BLOCKBEGIN)) {
    parseerror("Expected '{' block begin\n");
    return p->status = PARSE_ERR;
  }
  struct Token block_begin = { .type = T_BLOCK };
  ast_add_node(orig_branch, block_begin);
  next_token(p->lexer); // Skip '{'
  Ast block_branch = ast_get_last(orig_branch);
  p->ast = &block_branch;
  block(p);
  p->ast = orig_branch;
  p->loop--;  // Exit this loop block
  return NO_ERR;
}

// return ;
// return (expr) ;
// Output: { (expr) return }
// TODO: Fix empty return statements
int returnstat(struct Parser* p) {
  struct Token return_node = get_token(p->lexer);
  next_token(p->lexer); // Skip 'return'
  expr(p, 0);
  ast_add_node(p->ast, return_node);
  return NO_ERR;
}

// import filename
int importstat(struct Parser* p) {
  struct Token token = next_token(p->lexer);
  next_token(p->lexer);
  if (token.type != T_STRING) {
    parseerror("Expected string\n");
    return p->status = PARSE_ERR;
  }
  char path[PATH_LENGTH_MAX] = {0};
  snprintf(path, PATH_LENGTH_MAX, "%.*s", token.length, token.string);
  char* input = read_file(path);
  if (!input) {
    parseerror("'%s': No such file\n", path);
    return p->status = PARSE_ERR;
  }
  int status = parser_parse(input, p->str_arr, path, p->ast);
  free(input);
  return status;
}

int loadstat(struct Parser* p) {
  struct Token token = get_token(p->lexer);
  ast_add_node(p->ast, token);
  struct Token path_token = next_token(p->lexer);
  if (path_token.type != T_STRING) {
    parseerror("Expected string\n");
    return p->status = PARSE_ERR;
  }
  ast_add_node(p->ast, token);
  return NO_ERR;
}

int params(struct Parser* p) {
  struct Token token = get_token(p->lexer);
  if (token.type == T_CLOSEDPAREN)  // In case the function has no parameters
    return NO_ERR;
  for (;;) {
    struct Token token = get_token(p->lexer);
    switch (token.type) {
      case T_IDENTIFIER: {
        add_current_token(p);
        if (expect(p, T_CLOSEDPAREN)) {
          return NO_ERR;
        }
        else if (expect(p, T_COMMA))
          next_token(p->lexer);
        break;
      }

      default:
        parseerror("Invalid token in paramlist\n");
        return p->status = PARSE_ERR;
    }
  }
  return NO_ERR;
}

int arglist(struct Parser* p, int* num_args) {
  struct Token token = get_token(p->lexer);
  if (token.type == T_CLOSEDPAREN)
    return NO_ERR;
  for (;;) {
    (*num_args)++;
    expr(p, 0);
    if (expect(p, T_COMMA)) {
      next_token(p->lexer);
      continue;
    }
    return NO_ERR;
  }
  return NO_ERR;
}

// Ast output:
// fn
// identifier
//  \--> ( parameter list )
// T_BLOCK
//  \--> { BLOCK }
int funcstat(struct Parser* p) {
  Ast* orig_branch = p->ast;
  add_current_token(p); // Add and skip 'fn'
  if (!expect(p, T_IDENTIFIER)) {
    parseerror("Expected identifier\n");
    return p->status = PARSE_ERR;
  }
  add_current_token(p); // Add 'identifier'
  // Parse paramlist
  if (expect(p, T_OPENPAREN)) {
    next_token(p->lexer); // Skip '('
    Ast paramlist_branch = ast_get_last(orig_branch);
    p->ast = &paramlist_branch;
    params(p);
    if (!expect(p, T_CLOSEDPAREN)) {
      parseerror("Expected ')'\n");
      return p->status = PARSE_ERR;
    }
    next_token(p->lexer); // Skip ')'
    p->ast = orig_branch;
  }
  if (!expect(p, T_BLOCKBEGIN)) {
    parseerror("Expected '{'\n");
    return p->status = PARSE_ERR;
  }
  next_token(p->lexer); // Skip '{'
  struct Token block_begin = { .type = T_BLOCK };
  ast_add_node(orig_branch, block_begin);
  Ast block_branch = ast_get_last(orig_branch);
  p->ast = &block_branch;
  block(p);
  p->ast = orig_branch;
  return NO_ERR;
}

int block(struct Parser* p) {
  statements(p);
  if (!expect(p, T_BLOCKEND)) {
    parseerror("Expected '}' block end\n");
    return p->status = PARSE_ERR;
  }
  next_token(p->lexer); // Skip '}'
  return NO_ERR;
}

int breakstat(struct Parser* p) {
  struct Token token = get_token(p->lexer);
  next_token(p->lexer);
  if (!p->loop) {
    parseerror("No loop to break\n");
    return p->status = PARSE_ERR;
  }
  ast_add_node(p->ast, token);
  return NO_ERR;
}

int statement(struct Parser* p) {
  struct Token token = get_token(p->lexer);
  unsigned char expect_semicolon = 0;
  (void)expect_semicolon; // Unused; hide warning
  switch (token.type) {
    case T_EOF:
      return NO_ERR;

    case T_SEMICOLON:
      next_token(p->lexer);
      break;

    case T_DECL:
      declare_variable(p);
      expect_semicolon = 1;
      break;

    case T_FUNC_DEF:
      funcstat(p);
      break;

    case T_WHILE:
      whileloop(p);
      break;

    case T_BREAK:
      breakstat(p);
      expect_semicolon = 1;
      break;

    case T_IF:
      ifstatement(p);
      break;

    case T_RETURN:
      returnstat(p);
      expect_semicolon = 1;
      break;

    case T_IMPORT:
      importstat(p);
      expect_semicolon = 1;
      break;

    case T_LOAD:
      loadstat(p);
      expect_semicolon = 1;
      break;

    default:
      expr(p, 0);
      expect_semicolon = 1;
      break;
  }
  if (p->status != NO_ERR)
    return p->status;
  /*if (expect_semicolon) { // Temporary, might add a T_ENDSTATEMENT here to see where our statements ends
    if (!expect(p, T_SEMICOLON)) {
      parseerror("Expected ';'\n");
      return p->status = PARSE_ERR;
    }
    next_token(p->lexer);
  }*/
  if (expect(p, T_SEMICOLON))
    next_token(p->lexer);
  return p->status;
}

int statements(struct Parser* p) {
  while (!block_end(p))
    p->status = statement(p);
  return p->status;
}

// { identifier }
int postfix_expr(struct Parser* p) {
  struct Token token = get_token(p->lexer);
  switch (token.type) {
    case T_IDENTIFIER:
      next_token(p->lexer);
      if (expect(p, T_ASSIGN)) {  // Variable assignment?
        struct Token assign_token = get_token(p->lexer);
        next_token(p->lexer); // Skip '='
        expr(p, 0); // Parse the right hand side expression
        ast_add_node(p->ast, assign_token);
      }
      ast_add_node(p->ast, token); // Add identifier to ast
      break;

    case T_OPENPAREN: {
      next_token(p->lexer); // Skip '('
      expr(p, 0);
      if (!expression_end(p)) {
        parseerror("Missing ')' closing parenthesis in expression\n");
        return p->status = PARSE_ERR;
      }
      next_token(p->lexer); // Skip ')'
      break;
    }

    default: {
      parseerror("Unexpected symbol\n");
      next_token(p->lexer);
      return p->status = PARSE_ERR;
    }
  }
  // identifier '(' args ')'
  // (expr) '(' args ')'
  for (;;) {
    token = get_token(p->lexer);
    switch (token.type) {
      // T_CALL
      // \--> arglist
      // T_NUMBER (num_args)
      case T_OPENPAREN: {
        next_token(p->lexer);
        struct Token call_token = { .type = T_CALL };
        ast_add_node(p->ast, call_token);
        Ast* orig_branch = p->ast;
        Ast arglist_branch = ast_get_last(orig_branch);
        p->ast = &arglist_branch;
        int num_args = 0;
        arglist(p, &num_args);
        if (!expect(p, T_CLOSEDPAREN)) {
          parseerror("Missing ')' closing parenthesis in expression\n");
          return p->status = PARSE_ERR;
        }
        next_token(p->lexer);
        p->ast = orig_branch;
        struct Token num_args_token = (struct Token) {
          .type = T_NUMBER,
          .value.integer = num_args
        };
        ast_add_node(p->ast, num_args_token);
        break;
      }
      default:
        goto done;
    }
  }
done:
  return NO_ERR;
}

int simple_expr(struct Parser* p) {
  struct Token token = get_token(p->lexer);
  switch (token.type) {
    case T_SEMICOLON:
      next_token(p->lexer);
      break;

    case T_NUMBER:
      next_token(p->lexer);
      ast_add_node(p->ast, token);
      break;

    case T_STRING:
      next_token(p->lexer);
      ast_add_node(p->ast, token);
      break;

    default: {
      postfix_expr(p);
      return p->status;
    }
  }
  return p->status;
}

// Arithmetic operation on expressions
// expr op expr
int expr(struct Parser* p, int priority) {
  struct Token uop_token = get_token(p->lexer);
  int uop = get_uop(uop_token);
  if (uop != T_NOUNOP) {
    next_token(p->lexer); // Skip operator token
    expr(p, UNARY_PRIORITY);
    ast_add_node(p->ast, uop_token);
  }
  else {
    p->status = simple_expr(p);
  }

  struct Token token = get_token(p->lexer);
  int op = get_binop(token);
  while (op != T_NOBINOP && op_priority[op].left > priority) {
    int next_op;
    token = get_token(p->lexer);
    next_token(p->lexer);
    next_op = expr(p, op_priority[op].right);
    ast_add_node(p->ast, token);
    op = next_op;
  }
  return op;
}

int parser_parse(char* input, struct Str_arr* str_arr, const char* filename, Ast* ast) {
  strarr_append(str_arr, input);
  struct Lexer lexer = {
    .index = strarr_top(str_arr),
    .line = 1,
    .count = 0,
    .token = (struct Token) {0},
    .filename = filename,
  };
  struct Parser parser = {
    .lexer = &lexer,
    .ast = ast,
    .status = NO_ERR,
    .loop = 0,
    .str_arr = str_arr
  };
  next_token(parser.lexer);
  statements(&parser);
  check(&parser, T_EOF);
  return parser.status;
}
