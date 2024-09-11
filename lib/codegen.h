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
  ND_NUM, // integer
} NodeKind;

typedef struct _node_t node_t;

// abstruct syntax tree type
struct _node_t {
  NodeKind kind; // node type
  node_t *lhs; // left hand side expresstion
  node_t *rhs; // right hand side expresstion
  int val;
};

node_t *new_node(NodeKind kind, node_t *lhs, node_t *rhs);

node_t *new_node_num(int val);

// expr = equality
node_t *expr();

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
