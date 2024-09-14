#ifndef __PARSE_H__
#define __PARSE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

#include <tokenize.h>

// abstruct syntax tree kind
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_ADDR, // * pointer
  ND_DEREF, // & 
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
  ND_BLOCK, // {}
  ND_CALL, // call function statement
  ND_FUNC, // defined function statement
  ND_LVAR, // local variable
  ND_NUM, // integer
} NodeKind;

// type
typedef struct type_t type_t;

struct type_t {
  enum { INT, PTR } ty;
  struct type_t *ptr_to;
};

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

  // function statement
  char *name;
  node_t *arg;
  int argLen;
  node_t *stmt;

  // LVAR
  type_t *type;

  int val; // use if kind is ND_NUM
  int offset; // use if kind is ND_LVAR or ND_FUNCTION
};

typedef struct lvar_t lvar_t;

// local variable type
struct lvar_t {
  type_t *type; // type of variable
  char *name; // variable name
  int len; // length of variable name
  int offset; // offset from RBP
  lvar_t *next; // next variable or NULL
};

typedef struct func_t func_t;

// function type
struct func_t {
  type_t *type;
  char *name;
  func_t *next;
};

extern node_t *code[100];
extern lvar_t *locals;
extern int localLen;
extern func_t *funcs;

lvar_t *find_lvar(token_t *tok);

node_t *new_node(NodeKind kind, node_t *lhs, node_t *rhs);

node_t *new_node_num(int val);

node_t *new_node_lvar(token_t *tok);

node_t *program();

node_t *func();

node_t *new_block_stmt();

node_t *stmt();

node_t *expr();

node_t *assign();

node_t *equality();

node_t *relational();

node_t *add();

node_t *mul();

node_t *unary();

node_t *primary();

#endif
