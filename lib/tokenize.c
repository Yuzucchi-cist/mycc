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

token_t *peek(char *op) {
  if(token->kind != TK_RESERVED ||
    token->len != strlen(op) ||
    memcmp(token->str, op, token->len)
  ) {
    return NULL;
  }
  return token;
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

char *start_with_reserved(char *p) {
  // keywords
  static char *kw[] = {"return", "if", "else", "while", "for", "sizeof", "int", "char"};

  for(int i=0; i < sizeof(kw) / sizeof(*kw); i++) {
    int len = strlen(kw[i]);
    if(start_with(p, kw[i]) && !is_alnum(p[len]))
      return kw[i];
  }

  // multi-letter punctuator
  static char *ops[] = {"==", "!=", "<=", ">="};
  for(int i=0; i < sizeof(ops) / sizeof(*ops); i++) {
    int len = strlen(ops[i]);
    if(start_with(p, ops[i]))
      return ops[i];
  }

  return NULL;
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
    
    // keyword or multi-letter punctuator
    char *kw = start_with_reserved(p);
    if(kw) {
      int len = strlen(kw);
      cur = new_token(TK_RESERVED, cur, p, len);
      p+=len;
      continue;
    }
    
    // single-letter punctuator
    if(strchr("+-*/(){}[]<>=,&;", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
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

