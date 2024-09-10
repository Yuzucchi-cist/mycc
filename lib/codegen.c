#include "codegen.h"

node_t *new_node(NodeKind kind, node_t *lhs, node_t *rhs) {
  node_t *node = calloc(1, sizeof(node_t));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

node_t *new_node_num(int val) {
  node_t *node = calloc(1, sizeof(node_t));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

node_t *expr() {
  node_t *node = mul();

  for(;;) {
    if(consume('+')) {
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume('-')) {
      node = new_node(ND_SUB, node, mul());
    }
    else
      return node;
  }
}

node_t *mul() {
  node_t *node = unary();
  
  for(;;) {
    if(consume('*')) {
      node = new_node(ND_MUL, node, unary());
    }
    else if(consume('/')) {
      node = new_node(ND_DIV, node, unary());
    }
    else
      return node;
  }
}

node_t *unary() {
  if(consume('+'))
    return primary();
  if(consume('-'))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

node_t *primary() {
  // if next token is '(', next node would be `( <expr> )`
  if(consume('(')) {
    node_t *node = expr();
    expect(')');
    return node;
  }
  
  // other, node would be number
  return new_node_num(expect_number());
}


void gen(node_t *node) {
  if(node->kind == ND_NUM) {
    printf("\tpush %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("\tpop rdi\n");
  printf("\tpop rax\n");

  switch(node->kind) {
    case ND_ADD:
      printf("\tadd rax, rdi\n");
      break;
    case ND_SUB:
      printf("\tsub rax, rdi\n");
      break;
    case ND_MUL:
      printf("\timul rax, rdi\n");
      break;
    case ND_DIV:
      printf("\tcqo\n");
      printf("\tidiv rdi\n");
      break;
  }

  printf("\tpush rax\n");
}
