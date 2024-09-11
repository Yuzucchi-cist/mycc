#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include <parse.h>

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
  ND_LVAR, // local variable
  ND_NUM, // integer
} NodeKind;

typedef struct _node_t node_t;

// abstruct syntax tree type
struct _node_t {
  NodeKind kind; // node type
  node_t *lhs; // left hand side expresstion
  node_t *rhs; // right hand side expresstion
  int val; // use if kind is ND_NUM
  int offset; // use if kind is ND_LVAR
};

node_t *new_node(NodeKind kind, node_t *lhs, node_t *rhs);

node_t *new_node_num(int val);

extern node_t *code[100];

// program = stmt*
node_t *program();

// stmt = expr ";"
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

void gen(node_t *node);

#endif
