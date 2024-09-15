#include "tokenize.h"

token_t *token;

// if next token is expected token, returns true and read next token
// other, return false
bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
    token->len != strlen(op) ||
    memcmp(token->str, op, token->len)
  )
    return false;
  token = token->next;
  return true;
}

// if next token is expected statement, returns true and read next token
// other, return false
bool consume_statement(TokenKind kind) {
  if(token->kind == kind) {
    token = token->next;
    return true;
  }
  return false;
}

// if next token is identifier token, returns true and read next token
// other, return false
token_t *consume_ident() {
  if (token->kind != TK_IDENT) return NULL;
  token_t *tok = token;
  token = token->next;
  return tok;
}

// if next token is expected token, returns true and read next token
// other, throws error
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
    token->len != strlen(op) ||
    memcmp(token->str, op, token->len)
  )
    error_at(token->str, "'%s' is not expected symbol", op);
  token = token->next;
}

// if next token is number, returns the number and read next token
// other, throws error
int expect_number() {
  if (token->kind != TK_NUM) error_at(token->str, "'%s' is not number", token->str);
  int val = token->val;
  token = token->next;
  return val;
}

bool start_with(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

// is token charactor(alphabet, number, or underscore
bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// create new token and link the token to cur
token_t *new_token(TokenKind kind, token_t *cur, char *str, int len) {
  token_t *tok = calloc(1, sizeof(token_t));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool is_statement(char *p, char *statement) {
  int len = strlen(statement);
  return !strncmp(p, statement, len) && !is_alnum(p[len]);
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
    
    // multi-letter punctuator
    if(start_with(p, "==") ||
      start_with(p, "!=") ||
      start_with(p, "<=") ||
      start_with(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p+=2;
      continue;
    }
    
    // single-letter punctuator
    if(strchr("+-*/(){}<>=,&;", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(start_with(p, "int")) {
      cur = new_token(TK_RESERVED, cur, p, 3);
      p+=3;
      continue;
    }
    
    // integer literal
    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // sizeof statement
    if(is_statement(p, "sizeof")) {
      cur = new_token(TK_SIZEOF, cur, p, 6);
      p += 6;
      continue;
    }
    
    // return statement
    if(is_statement(p, "return")) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    // control statement
    if(is_statement(p, "if")) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if(is_statement(p, "else")) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if(is_statement(p, "while")) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if(is_statement(p, "for")) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    // alphabet literal
    if('a' <= *p && *p <= 'z') {
      int lvar_len = 0;
      for(char *q = p; 'a' <= *q && *q <= 'z' && *q != '\0'; q++) lvar_len++;

      cur = new_token(TK_IDENT, cur, p, lvar_len);
      p += lvar_len;
      continue;
    }

    error("cannot tokenize");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

