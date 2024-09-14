#include "parse.h"

lvar_t *locals = NULL;
func_t *funcs = NULL;

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

node_t *new_node_lvar(token_t *tok) {
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

node_t *code[100];

node_t *program() {
  int i=0;
  while(!at_eof())
    code[i++] = func();
//    code[i++] = stmt();
  code[i] = NULL;
}

// func = ident "(" ( (ident ",")* ident )? ")" "{" stmt "}"
node_t *func() {
  node_t *node = calloc(1, sizeof(node_t));
  node->kind = ND_FUNC;

  token_t *funcTok = consume_ident();
  node->name = calloc(1, sizeof(char) * funcTok->len);
  strncpy(node->name, funcTok->str, funcTok->len);
  node->name[funcTok->len] = '\0';

  expect("(");
  if(!consume(")")) {
    node_t *arg = node;
    for(;;) {
      arg->arg = new_node_lvar(consume_ident());
      arg = arg->arg;
      node->argLen++;
      if(consume(")"))  break;
      else expect(",");
    }
    arg->arg = NULL;
  }

  expect("{");
  node->stmt = new_block_stmt();
  
  if(funcs==NULL) funcs = calloc(1, sizeof(func_t));
  else {
    func_t *f = calloc(1, sizeof(func_t));
    f->next = funcs;
    funcs = f;
  }
  funcs->name = node->name;

  return node;
}

node_t *new_block_stmt() {
  node_t *node = calloc(1, sizeof(node_t));
  node->kind = ND_BLOCK;
  
  node_t *bstmt = node;

  while(!consume("}")) {
    bstmt->stmt = stmt();
    bstmt = bstmt->stmt;
  }
  bstmt->stmt = NULL;
  return node;
}

// stmt = expr ";"
//      | "{" stmt* "}"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr ";" expr ";" expr ")" stmt
node_t *stmt() {
  node_t *node;

  if(consume("{")) {
    node = new_block_stmt();
  } else if(consume_statement(TK_RETURN)) {
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

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
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

// add = mul ("+" mul | "-" mul)*
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

// mul = unary ("*" unary | "/" unary)*
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

// unary = ("+" | "-")? primary
node_t *unary() {
  if(consume("+"))
    return primary();
  if(consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

// primary = num
//         | ident "(" expr* ")"?
//         | "(" expr ")"
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
    // if next token of identifier is '(', next node would be function
    if(consume("(")) {
      node_t *node = calloc(1, sizeof(node_t));
      node->kind = ND_CALL;
      if(!consume(")")) {
        node_t *arg = node;
        for(;;) {
          arg->arg = expr();
          arg = arg->arg;
          if(consume(")"))  break;
          else  expect(",");
        }
      }
      node->name = calloc(1, sizeof(char) * tok->len);
      strncpy(node->name, tok->str, tok->len);
      node->name[tok->len] = '\0';
      return node;
    }

    return new_node_lvar(tok);
  }
  
  // other, node would be number
  return new_node_num(expect_number());
}

