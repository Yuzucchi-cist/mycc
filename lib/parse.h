#ifndef __PARSE_H__
#define __PARSE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

#include <tokenize.h>

typedef struct type_t type_t;
typedef struct var_t var_t;
typedef struct _node_t node_t;
typedef struct func_t func_t;

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
  ND_VAR, // variable
  ND_NUM, // integer
} NodeKind;

// type
struct type_t {
  enum { CHAR, INT, PTR, ARRAY } ty;
  struct type_t *ptr_to;
  int array_size;
};

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

  // VAR
  type_t *type;

  var_t *var;

  int val; // use if kind is ND_NUM
  int offset; // use if kind is ND_LVAR or ND_FUNCTION
};

// variable type
struct var_t {
  type_t *type; // type of variable
  char *name; // variable name
  bool is_local;  // is local value

  int len; // length of variable name
  int offset; // offset from RBP
  var_t *next;
};

// function type
struct func_t {
  type_t *type;
  char *name;
  func_t *next;
};

extern node_t *code[100];
extern var_t *globals;
extern var_t *locals;
extern int localLen;
extern func_t *funcs;

var_t *find_var(token_t *tok);

var_t *declare();

node_t *new_node(NodeKind kind);

node_t *new_binary(NodeKind kind, node_t *lhs, node_t *rhs);

node_t *new_var(var_t *var);

node_t *new_node_num(int val);

int size_of(type_t *ty);

type_t* type_specifier();

type_t *type_postfix(type_t *type);

type_t* type_name(type_t *type, char **name);

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

node_t *postfix();

node_t *primary();

#endif
