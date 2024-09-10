#include "parse.h"

token_t *token;

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
      *p == '(' || *p == ')' ||
      ) {
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
