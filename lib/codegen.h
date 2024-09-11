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

node_t *mul();
node_t *primary();
node_t *unary();

node_t *expr();

node_t *equality();

node_t *relational();

node_t *add();

node_t *mul();

node_t *unary();

node_t *primary();

void gen(node_t *node);

#endif
