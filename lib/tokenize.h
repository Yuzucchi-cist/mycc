#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

// kind of token
typedef enum {
  TK_RESERVED, // symbol
  TK_IDENT, // identifier
  TK_RETURN, // return
  TK_IF, // if
  TK_ELSE, // else
  TK_WHILE, // while
  TK_FOR, // for
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

bool consume(char *op);

bool consume_statement(TokenKind statement);

token_t *consume_ident();

void expect(char *op);

int expect_number();

bool start_with(char *p, char *q);

bool is_alnum(char c);

bool at_eof();

token_t *new_token(TokenKind kind, token_t *cur, char *str, int len);

token_t *tokenize(char *p);

#endif
