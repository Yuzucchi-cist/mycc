#ifndef __PARSE_H__
#define __PARSE_H__

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

// kind of token
typedef enum {
  TK_RESERVED, // symbol
  TK_IDENT, // identifier
  TK_RETURN, // return
  TK_IF, // if
  TK_ELSE, // else
  TK_WHILE, // while
  TK_FOR, // for
  TK_NUM, // integer
  TK_EOF, // token presented input end
} TokenKind;

// token type
typedef struct _token_t token_t;
struct _token_t {
  TokenKind kind; // token type
  token_t *next; // next token
  int val; // value if kind is TK_NUM
  char *str; // token charactors
  int len; // length of token charactors
};

extern token_t *token;

typedef struct lvar_t lvar_t;

// local variable type
struct lvar_t {
  char *name; // variable name
  int len; // length of variable name
  int offset; // offset from RBP
  lvar_t *next; // next variable or NULL
};

// local variable
extern lvar_t *locals;

// if next token is expected token, returns true and read next token
// other, return false
bool consume(char *op);

// if next token is identifier token, returns true and read next token
// other, return false
token_t *consume_ident();

// if next token is expected token, returns true and read next token
// other, throws error
void expect(char *op);

// if next token is number, returns the number and read next token
// other, throws error
int expect_number();

bool start_with(char *p, char *q);

// is token charactor(alphabet, number, or underscore
bool is_alnum(char c);

bool at_eof();

// create new token and link the token to cur
token_t *new_token(TokenKind kind, token_t *cur, char *str, int len);

// tokenize input string p and return it
token_t *tokenize(char *p);

// find variable by name
// return null if variable is not found
lvar_t *find_lvar(token_t *tok);

// abstruct syntax tree kind
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ, // ==
  ND_NE, // !=
  ND_LT, // <
  ND_LE, // <=
  ND_ASSIGN, // =
  ND_RETURN, // return statement
  ND_IF, // if statement
  ND_ELSE, // else statement
  ND_WHILE, // while statement
  ND_FOR, // for statement
  ND_LVAR, // local variable
  ND_NUM, // integer
} NodeKind;

typedef struct _node_t node_t;

// abstruct syntax tree type
struct _node_t {
  NodeKind kind; // node type
  node_t *lhs; // left hand side expresstion
  node_t *rhs; // right hand side expresstion

  // if or for statement
  node_t *cond;
  node_t *then;
  node_t *els;
  node_t *init;
  node_t *adv;

  int val; // use if kind is ND_NUM
  int offset; // use if kind is ND_LVAR
};


node_t *new_node(NodeKind kind, node_t *lhs, node_t *rhs);

node_t *new_node_num(int val);

extern node_t *code[100];

// program = stmt*
node_t *program();

// stmt = expr ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
node_t *stmt();

// expr = assign
node_t *expr();

// assign = equality ("=" assign)?
node_t *assign();

// equality = relational ("==" relational | "!=" relational)*
node_t *equality();

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
node_t *relational();

// add = mul ("+" mul | "-" mul)*
node_t *add();

// mul = unary ("*" unary | "/" unary)*
node_t *mul();

// unary = ("+" | "-")? primary
node_t *unary();

// primary = num | "(" expr ")"
node_t *primary();

#endif
