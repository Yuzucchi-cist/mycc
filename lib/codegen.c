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

node_t *new_node_lvar(char str) {
  node_t *node = calloc(1, sizeof(node_t));
  node->kind = ND_LVAR;
  node->offset = (str - 'a' + 1) * 8;
  return node;
}

node_t *code[100];

node_t *program() {
  int i=0;
  while(!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}

// stmt = expr ";"
node_t *stmt() {
  node_t *node = expr();
  expect(";");
  return node;
}

// expr = assign
node_t *expr() {
  return assign();
}

// assign = equality ("=" assign)?
node_t *assign() {
  node_t *node = equality();

  if(consume("="))
    return new_node(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
node_t *equality() {
  node_t *node = relational();
  NodeKind kind;

  if(consume("=="))
    return new_node(ND_EQ, node, relational());
  else if(consume("!="))
    return new_node(ND_NE, node, relational());
  else
    return node;
}

node_t *relational() {
  node_t *node = add();

  if(consume("<="))
    return new_node(ND_LE, node, add());
  else if(consume("<"))
    return new_node(ND_LT, node, add());
  else if(consume(">="))
    return new_node(ND_LE, add(), node);
  else if(consume(">"))
    return new_node(ND_LT, add(), node);
  else
    return node;
}

node_t *add() {
  node_t *node = mul();

  for(;;) {
    if(consume("+")) {
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume("-")) {
      node = new_node(ND_SUB, node, mul());
    }
    else
      return node;
  }
}

node_t *mul() {
  node_t *node = unary();
  
  for(;;) {
    if(consume("*")) {
      node = new_node(ND_MUL, node, unary());
    }
    else if(consume("/")) {
      node = new_node(ND_DIV, node, unary());
    }
    else
      return node;
  }
}

node_t *unary() {
  if(consume("+"))
    return primary();
  if(consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

node_t *primary() {
  // if next token is '(', next node would be `( <expr> )`
  if(consume("(")) {
    node_t *node = expr();
    expect(")");
    return node;
  }

  // if next token is identifier
  token_t *tok = consume_ident();
  if(tok) return new_node_lvar(tok->str[0]);
  
  // other, node would be number
  return new_node_num(expect_number());
}

void gen_lval(node_t *node) {
  if (node->kind != ND_LVAR)
    error("lhs of assignment is not variable");
  printf("\tmov rax, rbp\n");
  printf("\tsub rax, %d\n", node->offset);
  printf("\tpush rax\n");
}


void gen(node_t *node) {
  switch(node->kind) {
    case ND_NUM:
      printf("\tpush %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("\tpop rax\n");
      printf("\tmov rax, [rax]\n");
      printf("\tpush rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("\tpop rdi\n");
      printf("\tpop rax\n");
      printf("\tmov [rax], rdi\n");
      printf("\tpush rdi\n");
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
    case ND_EQ:
      printf("\tcmp rax, rdi\n");
      printf("\tsete al\n");
      printf("\tmovzb rax, al\n");
      break;
    case ND_NE:
      printf("\tcmp rax, rdi\n");
      printf("\tsetne al\n");
      printf("\tmovzb rax, al\n");
      break;
    case ND_LT:
      printf("\tcmp rax, rdi\n");
      printf("\tsetl al\n");
      printf("\tmovzb rax, al\n");
      break;
    case ND_LE:
      printf("\tcmp rax, rdi\n");
      printf("\tsetle al\n");
      printf("\tmovzb rax, al\n");
      break;
      
  }

  printf("\tpush rax\n");
}
