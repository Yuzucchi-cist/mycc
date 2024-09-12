#include "parse.h"

token_t *token;
lvar_t *locals;

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
  if (token->kind != TK_IDENT) return 0;
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
    if(strchr("+-*/()<>=;", *p)) {
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

// find variable by name
// return null if variable is not found
lvar_t *find_lvar(token_t *tok) {
  for(lvar_t *var = locals; var; var = var->next)
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

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
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr ";" expr ";" expr ")" stmt
node_t *stmt() {
    node_t *node;
  if(consume_statement(TK_RETURN)) {
    node = calloc(1, sizeof(node_t));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
  } else if(consume_statement(TK_IF)) {
    node = calloc(1, sizeof(node_t));
    node->kind = ND_IF;

    consume("(");
    node->cond = expr();
    consume(")");

    node->then = stmt();

    if(consume_statement(TK_ELSE))
      node->els = stmt();
    else  node->els = NULL;
  } else if(consume_statement(TK_WHILE)) {
    node = calloc(1, sizeof(node_t));
    node->kind = ND_WHILE;

    consume("(");
    node->cond = expr();
    consume(")");
    
    node->then = stmt();
  } else if(consume_statement(TK_FOR)) {
    node = calloc(1, sizeof(node_t));
    node->kind = ND_FOR;
    
    consume("(");
    node->init = expr();
    consume(";");
    node->cond = expr();
    consume(";");
    node->adv = expr();
    consume(")");

    node->then = stmt();
  } else {
    node = expr();
    expect(";");
  }

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
  if(tok) {
    node_t *node = calloc(1, sizeof(node_t));
    node->kind = ND_LVAR;

    lvar_t *lvar = find_lvar(tok);
    if(lvar)
      node->offset = lvar->offset;
    else {
      lvar = calloc(1, sizeof(lvar));
      lvar->name = tok->str;
      lvar->len = tok->len;
      if(!locals) lvar->offset = 8;
      else lvar->offset = locals->offset + 8;
      node->offset = lvar->offset;
      lvar->next = locals;
      locals = lvar;
    }
    return node;
  }
  
  // other, node would be number
  return new_node_num(expect_number());
}
