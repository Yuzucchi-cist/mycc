#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// make error function
// args are same as printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
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
  if (token->kind != TK_RESERVED || token->str[0] != op) error("'%c' is not expected symbol", op);
  token = token->next;
}

// if next token is number, returns the number and read next token
// other, throws error
int expect_number() {
  if (token->kind != TK_NUM) error("'%c' is not number", token->str);
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

    if(*p == '+' || *p == '-') {
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

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "ERROR!: Number of args == 2");
    return 1;
  }

  token = tokenize(argv[1]);

  // output a first half of assembry
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // check if first expresstion is number and output mov instruction
  printf("\tmov rax, %d\n", expect_number());

  // consume token of `+ <num>` or `- <num>` and output it
  while(!at_eof()) {
    if(consume('+')) {
      printf("\tadd rax, %d\n", expect_number());
      continue;
    }
    expect('-');
    printf("\tsub rax, %d\n", expect_number());
  }

  printf("\tret\n");
  return 0;
}
