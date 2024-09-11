#ifndef __PARSE_H__
#define __PARSE_H__

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

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
  int len; // length of token charactors
};

extern token_t *token;

// if next token is expected token, returns true and read next token
// other, return false
bool consume(char *op);

// if next token is expected token, returns true and read next token
// other, throws error
void expect(char *op);

// if next token is number, returns the number and read next token
// other, throws error
int expect_number();

bool start_with(char *p, char *q);

bool at_eof();

// create new token and link the token to cur
token_t *new_token(TokenKind kind, token_t *cur, char *str, int len);

// tokenize input string p and return it
token_t *tokenize(char *p);
#endif
