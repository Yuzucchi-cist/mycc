#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// input program string
char *user_input;

// kind of token
typedef enum {
  TK_RESERVED, // symbol
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
};

 token_t *token;

// print error
// args are same as printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// print error with place
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}


// if next token is expected token, returns true and read next token
// other, return false
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) return false;
  token = token->next;
  return true;
}

// if next token is expected token, returns true and read next token
// other, throws error
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) error_at(token->str, "'%c' is not expected symbol", op);
  token = token->next;
}

// if next token is number, returns the number and read next token
// other, throws error
int expect_number() {
  if (token->kind != TK_NUM) error_at(token->str, "'%c' is not number", token->str);
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// create new token and link the token to cur
token_t *new_token(TokenKind kind, token_t *cur, char *str) {
  token_t *tok = calloc(1, sizeof(token_t));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// tokenize input string p and return it
token_t *tokenize(char *p) {
  token_t head;
  head.next = NULL;
  token_t *cur = &head;

  while(*p) {
    // skip space
    if(isspace(*p)) {
      p++;
      continue;
    }

    if(*p == '+' || *p == '-' ||
      *p == '*' || *p == '/' ||
      *p == '(' || *p == ')') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("cannot tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

// abstruct syntax tree kind
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
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

node_t *mul();
node_t *primary();

node_t *expr() {
  node_t *node = mul();

  for(;;) {
    if(consume('+')) {
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume('-')) {
      node = new_node(ND_SUB, node, mul());
    }
    else if(consume('*')) {
      node = new_node(ND_MUL, node, mul());
    }
    else if(consume('/')) {
      node = new_node(ND_DIV, node, mul());
    }
    else
      return node;
  }
}

node_t *mul() {
  node_t *node = primary();
  
  for(;;) {
    if(consume('*')) {
      node = new_node(ND_MUL, node, primary());
    }
    else if(consume('/')) {
      node = new_node(ND_DIV, node, primary());
    }
    else
      return node;
  }
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

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "ERROR!: Number of args == 2");
    return 1;
  }

  // tokenize and parse
  user_input = argv[1];
  token = tokenize(user_input);
  node_t *node = expr();

  // output a first half of assembry
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  
  gen(node);

  // load stack top that is expresstion value to rax register as return value
  printf("\tpop rax\n");
  printf("\tret\n");
  return 0;
}
