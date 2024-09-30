#include "parse.h"

var_t *locals = NULL;
var_t *globals = NULL;
func_t *funcs = NULL;
int localOffset = 0;

// find variable by name
// return null if variable is not found
var_t *find_var(token_t *tok) {
  for(var_t *var = locals; var; var = var->next)
    if(tok->len == var->len && !memcmp(tok->str, var->name, tok->len))
      return var;

  for(var_t *var = globals; var; var = var->next)
    if(tok->len == var->len && !memcmp(tok->str, var->name, tok->len))
      return var;
  return NULL;
}

var_t *push_var(type_t *ty, char *name, bool is_local, token_t *tok) {
  var_t *var = calloc(1, sizeof(var_t));
  var->type = ty;
  var->name = name;
  var->len = tok->len;

  var->is_local = is_local;
  var->tok = tok;

  if(is_local) {
    int offset = size_of(ty);
    if(!locals) var->offset = offset;
    else  var->offset = localOffset + offset;
    localOffset += offset;

    var->next = locals;
    locals = var;
  } else {
    var->next = globals;
    globals = var;
  }

  return var;
}

// add global variable
// global_var = type_name type_postfix ident ( = global_initializer )? ";"
void global_var() {
  type_t *ty = type_specifier();
  char *name;
  token_t *tok;
  ty = type_name(ty, &name, &tok);
  ty = type_postfix(ty);

  var_t *var = push_var(ty, name, false, tok);

  if(consume("=")) {
    initializer_t *init = global_initializer(ty);
    var->init = init;
  }
  
  expect(";");
}
  
// add local variarable
var_t *add_var(type_t *ty, token_t *tok) {
  
  var_t *var = push_var(ty, tok->str, true, tok);
  return var;
}

var_t *declare() {
  type_t *ty = type_specifier();
  token_t *tok = consume_ident();
  ty = type_postfix(ty);

  char *name = calloc(1, sizeof(char) * tok->len + 1 );
  memcpy(name, tok->str, tok->len);
  name[tok->len] = '\0';

  var_t *var = find_var(tok);
  if(var) {
    error_at(false, var->tok->str, "'%s' is already declarated here", name);
    error_at(true, tok->str, "redeclaration of '%s'", name);
  }

  return push_var(ty, name, true, tok);
}

bool is_type() {
  return peek("int") || peek("char");
}

// global_initializer = str | expr
initializer_t *global_initializer(type_t *ty) {
  // str
  token_t *tok = consume_kind(TK_STR);
  if(ty->ptr_to && ty->ptr_to->ty == CHAR && tok) {
    initializer_t *head = calloc(1, sizeof(initializer_t));
    head->next = NULL;
    initializer_t *cur = head;

    int i = 0;

    if(ty->is_completed && ty->array_size < tok->len)
      error_at(true, tok->str, "array is too long");

    do {
      cur = new_init_val(cur, tok->str[i]);
      i++;
    } while(i<tok->len);
    cur = new_init_val(cur, '\0');

    if(ty->is_completed)  cur = new_init_zero(cur, ty->array_size - i);
    else  ty->array_size = tok->len;
    
    ty->ty = ARRAY;

    return head->next;
  }

  if(tok) error_at(true, tok->str, "literal is not valid");

  if(ty->ty == ARRAY) {
    consume("{");

    int limit = ty->is_completed ? ty->array_size : INT_MAX;
    initializer_t *head = calloc(1, sizeof(initializer_t));
    head->next = NULL;
    initializer_t *cur = head;
    int i=0;
    do {
       cur->next = global_initializer(ty->ptr_to);
       cur = cur->next;
       i++;
    } while(i<limit && consume(","));

    if(i>=limit)
      error_at(true, token->str, "array is too long");

    consume("}");

    if(ty->is_completed)
      cur = new_init_zero(cur, ty->array_size - i);
    else
      ty->array_size = i;

    return head->next;
  }

  node_t *e = expr();

  var_t *var = NULL;
  int addend = eval(e, &var);
  if(var)
    return new_init_label(NULL, var->name, addend);

  return new_init_val(NULL, addend);
}

node_t *local_initializer(node_t *var_node) {
  type_t *ty = var_node->type;
  if(consume("{")) {

    int i=0;
    int limit = ty->is_completed ? ty->array_size : INT_MAX;

    node_t *head = calloc(1, sizeof(node_t));
    head->stmt = NULL;
    node_t *cur = head;
    node_t *tmp;
    do {
      tmp = new_node(ND_DEREF);
      tmp->lhs = new_binary(ND_ADD, var_node, new_node_num(i));
      tmp->type = tmp->lhs->type;

      cur->stmt = local_initializer(tmp);
      cur = cur->stmt;
      i++;
    } while(i<limit && consume(","));

    if(!ty->is_completed)  ty->array_size = i;

    if(i>ty->array_size)
      error_at(true, token->str, "array is too long");

    consume("}");

    return head->stmt;
  }
  return new_binary(ND_ASSIGN, var_node, assign());
}

initializer_t *new_init_val(initializer_t *cur, int val) {
  initializer_t *init = calloc(1, sizeof(initializer_t));
  init->val = val;
  if(cur) cur->next = init;
  return init;
}

initializer_t *new_init_label(initializer_t *cur, char *label, int addend) {
  initializer_t *init = calloc(1, sizeof(initializer_t));
  init->label = label;
  init->addend = addend;
  if(cur) cur->next = init;
  return init;
}

initializer_t *new_init_zero(initializer_t *cur, int size) {
  for(int i=0; i<size; i++) {
    cur->next = new_init_val(cur, 0);
    cur = cur->next;
  }
  return cur;
}

char *new_label() {
  static int cnt = 0;
  char *buf = calloc(1, sizeof(char) * 20);
  sprintf(buf, ".Lstr%d", cnt++);
  return buf;
}

node_t *new_node(NodeKind kind) {
  node_t *node = calloc(1, sizeof(node_t));
  node->kind = kind;
  return node;
}

node_t *new_binary(NodeKind kind, node_t *lhs, node_t *rhs) {
  node_t *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  node->type = node->lhs->type;
  return node;
}

node_t *new_var(var_t *var) {
  node_t *node = new_node(ND_VAR);
  node->var = var;
  node->type = var->type;
  return node;
}

node_t *new_node_num(int val) {
  node_t *node = calloc(1, sizeof(node_t));
  node->kind = ND_NUM;
  node->type = calloc(1, sizeof(type_t));
  node->type->ty = INT;
  node->val = val;
  return node;
}

node_t *new_node_str(token_t *tok) {
  type_t *ty = calloc(1, sizeof(type_t));
  ty->ty = ARRAY;
  ty->ptr_to = calloc(1, sizeof(type_t));
  ty->ptr_to->ty = CHAR;
  ty->array_size = tok->len;

  var_t *var = push_var(ty, new_label(), false, tok);
  var->str = calloc(1, sizeof(char) * tok->len + 1);
  strncpy(var->str, tok->str, tok->len);
  var->str[tok->len] = '\0';

  return new_var(var);
}

int size_of(type_t *ty) {
  switch(ty->ty) {
    case CHAR:
      return 1;
    case INT:
      return 4;
    case PTR:
      return 8;
    case ARRAY:
      return size_of(ty->ptr_to) * ty->array_size;
  }
}

node_t *code[100];

bool is_function() {
  token_t *tok = token;

  type_t *ty = type_specifier();
  char *name;
  type_name(ty, &name, NULL);

  if(ty->ty == ARRAY) return false;

  bool isFunc = ty && name && consume("(");
  token = tok;
  return isFunc;
}

node_t *program() {
  int i=0;
  while(!at_eof()) {
    if(is_function()) {
      code[i++] = func();
    } else {
      global_var();
    }
  }
  code[i] = NULL;
}

// type = int "*"*
type_t *type_specifier() {
  if(!is_type()) return NULL;
  type_t *ty = calloc(1, sizeof(type_t));

  if(consume("int")) {
    ty->ty = INT;
  } else if(consume("char")) {
    ty->ty = CHAR;
  }
  while(consume("*")) {
    type_t *ptr = calloc(1, sizeof(type_t));
    ptr->ty = PTR;
    ptr->ptr_to = ty;
    ty = ptr;
  }
  return ty;
}

type_t *type_postfix(type_t *ty) {
  for(;;) {
    if(consume("[")) {
      type_t *ptr = calloc(1, sizeof(type_t));
      ptr->ty = ARRAY;
      ptr->ptr_to = ty;
      ty = ptr;

      if(consume("]")) {
        ty->is_completed = false;
        continue;
      }

      ty->is_completed = true;

      ty->array_size = expect_number();
      expect("]");
    } else {
      return ty;
    }
  }
}

type_t *type_name(type_t *type, char **name, token_t **tok) {
  if(tok == NULL) tok = calloc(1, sizeof(token_t*));
  *tok = consume_ident();
  *name = calloc(1, sizeof(char) * (*tok)->len + 1);
  strncpy(*name, (*tok)->str, (*tok)->len);
  (*name)[(*tok)->len] = '\0';
  return type;
}
  

// func = int ident "(" ( (int ident ",")* int ident )? ")" "{" stmt "}"
node_t *func() {
  localOffset = 0;
  type_t *ty = type_specifier();
  char *name;
  ty = type_name(ty, &name, NULL);
  
  node_t *node = calloc(1, sizeof(node_t));
  node->kind = ND_FUNC;
  node->name = name;

  expect("(");
  locals = NULL;
  if(!consume(")")) {
    node_t *arg = node;
    for(;;) {
      
      
      arg->arg = new_var(declare());
      arg = arg->arg;
      node->argLen++;

      if(consume(")"))  break;
      else expect(",");
    }
    arg->arg = NULL;
  }

  expect("{");
  node->stmt = new_block_stmt();
  node->offset = localOffset;
  expect("}");
  
  if(funcs==NULL) funcs = calloc(1, sizeof(func_t));
  else {
    func_t *f = calloc(1, sizeof(func_t));
    f->next = funcs;
    funcs = f;
  }
  funcs->type = node->type;
  funcs->name = node->name;

  return node;
}

node_t *new_block_stmt() {
  node_t *node = calloc(1, sizeof(node_t));
  node->kind = ND_BLOCK;
  
  node_t *bstmt = node;
  var_t *oldLocals = locals;

  while(!peek("}")) {
    bstmt->stmt = stmt();
    while(bstmt->stmt)
      bstmt = bstmt->stmt;
  }
  bstmt->stmt = NULL;

  locals = oldLocals;

  return node;
}

// stmt = expr ";"
//      | "int" ident ("[" num "]")? ";"
//      | "{" stmt* "}"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr ";" expr ";" expr ")" stmt
node_t *stmt() {
  node_t *node;
  
  if(is_type()) {
    token_t *tok = token;
    var_t *var = declare();
    if(consume("=")) {
      node_t *node = local_initializer(new_var(var));
      expect(";");
      return node;
    }
    if(var->type->ty == ARRAY && !var->type->is_completed) {
      token=tok;
      type_specifier();
      error_at(true, token->str, "array size missing in %s", var->name);
    }
    expect(";");
    return NULL;
  }
  else if(consume("{")) {
    node = new_block_stmt();
    expect("}");
  } else if(consume("return")) {
    node = calloc(1, sizeof(node_t));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
  } else if(consume("if")) {
    node = calloc(1, sizeof(node_t));
    node->kind = ND_IF;

    consume("(");
    node->cond = expr();
    consume(")");

    node->then = stmt();

    if(consume("else"))
      node->els = stmt();
    else  node->els = NULL;
  } else if(consume("while")) {
    node = calloc(1, sizeof(node_t));
    node->kind = ND_WHILE;

    consume("(");
    node->cond = expr();
    consume(")");
    
    node->then = stmt();
  } else if(consume("for")) {
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
    return new_binary(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
node_t *equality() {
  node_t *node = relational();
  NodeKind kind;

  if(consume("=="))
    return new_binary(ND_EQ, node, relational());
  else if(consume("!="))
    return new_binary(ND_NE, node, relational());
  else
    return node;
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
node_t *relational() {
  node_t *node = add();

  if(consume("<="))
    return new_binary(ND_LE, node, add());
  else if(consume("<"))
    return new_binary(ND_LT, node, add());
  else if(consume(">="))
    return new_binary(ND_LE, add(), node);
  else if(consume(">"))
    return new_binary(ND_LT, add(), node);
  else
    return node;
}

// add = mul ("+" mul | "-" mul)*
node_t *add() {
  node_t *node = mul();

  for(;;) {
    if(consume("+")) {
      node = new_binary(ND_ADD, node, mul());
    }
    else if(consume("-")) {
      node = new_binary(ND_SUB, node, mul());
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
      node = new_binary(ND_MUL, node, unary());
    }
    else if(consume("/")) {
      node = new_binary(ND_DIV, node, unary());
    }
    else
      return node;
  }
}

// unary = "sizeof" unary
//       | "+" primary
//       | "-" primary
//       | "*" unary
//       | "&" unary
node_t *unary() {
  if(consume("sizeof")) {
    return new_node_num(size_of(unary()->type));
  }
  else if(consume("+"))
    return postfix();
  else if(consume("-"))
    return new_binary(ND_SUB, new_node_num(0), postfix());
  else if(consume("*")) {
    node_t *node = new_node(ND_DEREF);
    node->lhs =  unary();
    node->type = node->lhs->type;
    return node;
  }
  else if(consume("&")) {
    node_t *node = new_node(ND_ADDR);
    node->lhs =  unary();
    node->type = node->lhs->type;
    return node;
  }
  return postfix();
}


// postfix = primary ( "[" expr "]" )*
node_t *postfix() {
  node_t *node = primary();

  for(;;) {
    if(consume("[")) {
      node_t *exp = new_binary(ND_ADD, node, expr());
      expect("]");

      node = new_node(ND_DEREF);
      node->lhs =  exp;
      node->type = node->lhs->type;
      continue;
    } else {
      return node;
    }
  }
}

// primary = num
//         | str
//         | ident "(" expr* ")"?
//         | "(" ( expr | { stmt* } )  ")"
node_t *primary() {
  // if next token is '(', next node would be `( <expr> )`
  if(consume("(")) {
    node_t *node;
    if(consume("{")) {
      node = new_block_stmt();
      expect("}");
    } else {
      node = expr();
    }
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
      
      node->name = calloc(1, sizeof(char) * tok->len + 1);
      strncpy(node->name, tok->str, tok->len);
      node->name[tok->len] = '\0';
      node->type = calloc(1, sizeof(type_t));
      node->type->ty = INT;
      return node;
    }
    var_t *var = find_var(tok);
    if(!var) {
      char *str = tok->str;
      str[tok->len] = '\0';
      error_at(true, tok->str, "'%s' is not found", str);
    }
    return new_var(var);
  }

  // str
  tok = consume_kind(TK_STR);
  if(tok) return new_node_str(tok);
  
  // other, node would be number
  return new_node_num(expect_number());
}

int eval(node_t *node, var_t **var) {
  switch(node->kind) {
    case ND_ADD:
      return eval(node->lhs, var) + eval(node->rhs, var);
    case ND_SUB:
      return eval(node->lhs, var) - eval(node->rhs, NULL);
    case ND_MUL:
      return eval(node->lhs, NULL) * eval(node->rhs, NULL);
    case ND_DIV:
      return eval(node->lhs, NULL) / eval(node->rhs, NULL);
    case ND_EQ:
      return eval(node->lhs, NULL) == eval(node->rhs, NULL);
    case ND_NE:
      return eval(node->lhs, NULL) != eval(node->rhs, NULL);
    case ND_LT:
      return eval(node->lhs, NULL) < eval(node->rhs, NULL);
    case ND_LE:
      return eval(node->lhs, NULL) <= eval(node->rhs, NULL);
    case ND_NUM:
      return node->val;
    case ND_VAR:
      *var = node->var;
      return 0;
    case ND_ADDR:
      *var = node->lhs->var;
      return 0;
  }
}

